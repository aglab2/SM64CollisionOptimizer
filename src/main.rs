#[path = "../build/embedded_data.rs"]
mod embedded_data;

mod checksum;
mod patcher;

use egui::{Color32, Panel, RichText, ScrollArea, Ui, Vec2};
use std::path::PathBuf;
use anyhow::{Context, Result};

#[derive(Clone, PartialEq)]
struct CollisionConfig {
    wallkick_angle_raw: u16,
    num_quarter_steps: u16,
    normal_floor_ceil_threshold: f32,
}

/// Parse a wallkick angle string supporting both degrees and raw s16 values.
/// Accepts: "0x4000", "16384", "0x7fff"
fn parse_wallkick_angle(input: &str) -> Option<u16> {
    let trimmed = input.trim();
    if trimmed.is_empty() {
        return None;
    }

    // Try hex format: 0x4000
    if trimmed.starts_with("0x") || trimmed.starts_with("0X") {
        if let Ok(val) = u16::from_str_radix(&trimmed[2..], 16) {
            return Some(val);
        }
    }

    // Try raw decimal s16 value
    if let Ok(raw) = u16::from_str_radix(&trimmed, 16) {
        return Some(raw.min(0x7FFF));
    }

    None
}

impl CollisionConfig {
    /// Convert degrees to raw s16 angle value (DEGREES macro: degrees * 0x10000 / 360)
    fn degrees_to_raw(degrees: f64) -> u16 {
        ((degrees * 65536.0 / 360.0) as u16).min(0x7FFF)
    }

    /// Convert raw s16 angle value to degrees
    fn raw_to_degrees(raw: u16) -> f64 {
        raw as f64 * 360.0 / 65536.0
    }

    /// Pack into ROM bytes (big-endian, N64 format)
    fn to_rom_bytes(&self) -> [u8; 12] {
        let wallkick_fp = self.wallkick_angle_raw;
        let num_steps = self.num_quarter_steps as u16;
        let threshold = self.normal_floor_ceil_threshold;

        [
            (wallkick_fp >> 8) as u8, (wallkick_fp & 0xFF) as u8,
            (num_steps >> 8) as u8, (num_steps & 0xFF) as u8,
            threshold.to_be_bytes()[0], threshold.to_be_bytes()[1],
            threshold.to_be_bytes()[2], threshold.to_be_bytes()[3],
            0, 0, 0, 0, // padding to 12 bytes
        ]
    }
}

impl Default for CollisionConfig {
    fn default() -> Self {
        Self {
            wallkick_angle_raw: 0x2000,
            num_quarter_steps: 4,
            normal_floor_ceil_threshold: 0.05,
        }
    }
}

#[derive(Default)]
struct App {
    patched_path: Option<PathBuf>,
    patch_status: PatchStatus,
    collision_config: CollisionConfig,
}

#[derive(Default)]
enum PatchStatus {
    #[default]
    Idle,
    Success,
    Error(String),
}

fn main() -> eframe::Result {
    let options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default().with_inner_size(Vec2::new(300.0, 300.0)),
        ..Default::default()
    };
    eframe::run_native(
        "Better SM64 Collision Patcher",
        options,
        Box::new(|_cc| Ok(Box::new(App::default()))),
    )
}

impl eframe::App for App {
    fn ui(&mut self, ui: &mut Ui, _frame: &mut eframe::Frame) {
        // Top panel with file selection
        Panel::top("top_panel").show(ui, |ui| {
            ui.horizontal(|ui| {
                if ui.button("Open ROM").clicked() {
                    let (path, status) = self.open_rom_dialog();
                    self.patch_status = status;
                    self.patched_path = path;
                }
                if let Some(ref path) = self.patched_path {
                    if ui.button("Reveal Patched ROM").clicked() {
                        self.reveal_in_file_manager(path);
                    }
                }
            });
        });

        ScrollArea::both().show(ui, |ui| {
            let mut config = self.collision_config.clone();

            ui.group(|ui| {
                let wallkick_text = RichText::new("Wallkick Angle");
                ui.horizontal(|ui| {
                    ui.label(wallkick_text);
                    ui.label(RichText::new("?").color(Color32::LIGHT_BLUE).small())
                        .on_hover_ui(|ui| {
                            ui.set_max_width(220.0);
                            ui.label("The angle at which wallkicks activate. For 45 degree wallkicks, use 0x2001.");
                        });
                });

                let mut deg = CollisionConfig::raw_to_degrees(config.wallkick_angle_raw);
                ui.add(egui::Slider::new(&mut deg, 0.0..=90.0).suffix("°"));
                config.wallkick_angle_raw = CollisionConfig::degrees_to_raw(deg);

                let mut raw = config.wallkick_angle_raw;
                ui.add(egui::DragValue::new(&mut raw)
                    .custom_formatter(|v, _| format!("0x{:04X}", v as u32).into())
                    .custom_parser(|s| {
                        parse_wallkick_angle(&s).map(|v| v as f64)
                    })
                    .speed(256.0));
                config.wallkick_angle_raw = raw.min(0x7FFF);
            });

            ui.group(|ui| {
                let quarter_text = RichText::new("Quarter Steps");
                ui.horizontal(|ui| {
                    ui.label(quarter_text);
                    ui.label(RichText::new("?").color(Color32::LIGHT_BLUE).small())
                        .on_hover_ui(|ui| {
                            ui.set_max_width(220.0);
                            ui.label("Number of sub-steps for collision checks per frame. More steps = more precise collision detection but has worse performance");
                        });
                });
                ui.add(egui::Slider::new(&mut config.num_quarter_steps, 4..=16)
                    .suffix(" steps"));
            });

            ui.group(|ui| {
                let threshold_text = RichText::new("Normal Floor/Ceil Threshold");
                ui.horizontal(|ui| {
                    ui.label(threshold_text);
                    ui.label(RichText::new("?").color(Color32::LIGHT_BLUE).small())
                        .on_hover_ui(|ui| {
                            ui.set_max_width(220.0);
                            ui.label("Maximum slope angle to be considered a floor/ceiling instead of a wall. Lower values = more very steep floors detection. Vanilla is 0.01");
                        });
                });
                ui.add(egui::Slider::new(&mut config.normal_floor_ceil_threshold, 0.01..=0.08)
                    .suffix(""));
            });

            if config != self.collision_config {
                self.collision_config = config;
            }
        });

        Panel::bottom("bottom_panel").show(ui, |ui| {
            match &self.patch_status {
                PatchStatus::Idle => {
                    ui.label("Ready. Open a ROM file to begin.");
                }
                PatchStatus::Success => {
                    ui.label(RichText::new("ROM patched successfully!").color(Color32::GREEN),);
                    if let Some(ref path) = self.patched_path {
                        let name = path.file_name().map(|v| v.display().to_string());
                        ui.label(format!("Saved: {}", name.unwrap()));
                    }
                }
                PatchStatus::Error(msg) => {
                    ui.label(RichText::new(format!("Error: {}", msg)).color(Color32::RED),);
                }
            }
        });
    }
}

impl App {
    fn open_rom_dialog(&self) -> (Option<PathBuf>, PatchStatus) {
        let Some(path) = self.select_rom() else { return (None, PatchStatus::Idle) };
        let result = self.do_patch(path);

        match result {
            Ok(path) => {
                (Some(path), PatchStatus::Success)
            },

            Err(e) => {
                (None, PatchStatus::Error(format!("{}", e)))
            }
        }
    }

    fn do_patch(&self, path: PathBuf) -> Result<PathBuf> {
        let rom = self.open_rom(&path)
                         .with_context(|| format!("Failed to open ROM {}", path.to_string_lossy()))?;

        let mut patched_rom = self.patch_rom(rom)
                              .with_context(|| "Failed to patch ROM")?;

        // Write collision config to ROM at gCollisionConfig address (0xFFF50)
        let config_bytes = self.collision_config.to_rom_bytes();
        let config_addr = 0xFFF50usize;
        if config_addr + config_bytes.len() <= patched_rom.len() {
            patched_rom[config_addr..config_addr + config_bytes.len()].copy_from_slice(&config_bytes);
        }

        checksum::update_header_checksums(&mut patched_rom);

        let out_path = path.with_file_name(format!(
            "{}_patched.z64",
            path.file_stem().unwrap().to_string_lossy()
        ));

        std::fs::write(&out_path, patched_rom).with_context(|| format!("Failed to write patched ROM {}", out_path.display()))?;
        Ok(out_path)
    }

    fn select_rom(&self) -> Option<PathBuf> {
        rfd::FileDialog::new().add_filter("N64 ROM", &["z64"])
                              .add_filter("All files", &["*"])
                              .pick_file()
    }

    fn open_rom(&self, path: &PathBuf) -> Result<Vec<u8>> {
        Ok(std::fs::read(&path)?)
    }

    fn patch_rom(&self, mut rom_data: Vec<u8>) -> Result<Vec<u8>> {
        patcher::patch_rom(&mut rom_data)?;
        Ok(rom_data)
    }

    fn reveal_in_file_manager(&self, path: &PathBuf) {
        if !path.exists() {
            return;
        }
        #[cfg(target_os = "macos")]
        let _ = std::process::Command::new("open")
            .arg("-R")
            .arg(path)
            .spawn();
        #[cfg(target_os = "windows")]
        let _ = std::process::Command::new("explorer")
            .args(["/select," , path.to_str().unwrap()])
            .spawn();
        #[cfg(target_os = "linux")]
        {
            let file = path.to_str().unwrap();
            let args = vec!["open", "--select", file];
            if let Ok(output) = std::process::Command::new("gio")
                .args(&args)
                .output()
            {
                if output.status.success() {
                    return;
                }
            }

            // Last resort: open parent directory
            if let Some(parent) = path.parent() {
                let _ = std::process::Command::new("xdg-open")
                    .arg(parent)
                    .spawn();
            }
        }
    }
}
