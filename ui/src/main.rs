#[path = "../build/embedded_data.rs"]
mod embedded_data;

mod checksum;
mod patcher;

use egui::{Color32, Panel, RichText, ScrollArea, Ui, Vec2};
use std::path::PathBuf;

#[derive(Default)]
struct App {
    rom_path: Option<PathBuf>,
    rom_data: Option<Vec<u8>>,
    patched_path: Option<PathBuf>,
    patch_status: PatchStatus,
    expanded_functions: std::collections::HashSet<String>,
}

#[derive(Default)]
enum PatchStatus {
    #[default]
    Idle,
    Patching,
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
                    self.open_rom_dialog();
                    self.patch_rom();

                    if let Some(ref path) = self.rom_path {
                        let out_path = path.with_file_name(format!(
                            "{}_patched.z64",
                            path.file_stem().unwrap().to_string_lossy()
                        ));

                        if let Some(ref data) = self.rom_data {
                            if let Err(e) = std::fs::write(&out_path, data) {
                                self.patch_status = PatchStatus::Error(format!(
                                    "Failed to save: {}",
                                    e
                                ));
                            } else {
                                self.patched_path = Some(out_path);
                            }
                        }
                    }
                }
                if ui.button("Reveal Patched ROM").clicked() {
                    if let Some(ref path) = self.patched_path {
                        self.reveal_in_file_manager(path);
                    }
                }
            });
        });

        ScrollArea::both().show(ui, |ui| {
            // TODO: Planned configs
            ui.label("Hi!");
        });

        // Bottom panel for status
        Panel::bottom("bottom_panel").show(ui, |ui| {
            match &self.patch_status {
                PatchStatus::Idle => {
                    if self.rom_path.is_some() {
                        ui.label("ROM loaded. Click \"Open ROM\" to patch and save.");
                    } else {
                        ui.label("Ready. Open a ROM file to begin.");
                    }
                }
                PatchStatus::Patching => {
                    ui.label("Patching...");
                }
                PatchStatus::Success => {
                    ui.label(RichText::new("ROM patched successfully!").color(Color32::GREEN),);
                    if let Some(ref path) = self.patched_path {
                        ui.label(format!("Saved: {}. Click \"Reveal Patched ROM\" to locate it.", path.display()));
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
    fn open_rom_dialog(&mut self) {
        let file_result = rfd::FileDialog::new()
            .add_filter("N64 ROM", &["z64"])
            .add_filter("All files", &["*"])
            .pick_file();

        if let Some(path) = file_result {
            self.rom_path = Some(path.clone());
            match std::fs::read(&path) {
                Ok(data) => {
                    // Validate it looks like an N64 ROM
                    if data.len() >= 0x10 && data.len() % 8 == 0 {
                        self.rom_data = Some(data);
                        self.patch_status = PatchStatus::Idle;
                    } else {
                        self.patch_status = PatchStatus::Error(
                            "Invalid ROM file (bad size or header)".to_string(),
                        );
                    }
                }
                Err(e) => {
                    self.patch_status = PatchStatus::Error(format!("Failed to read ROM: {}", e));
                }
            }
        }
    }

    fn patch_rom(&mut self) {
        let mut rom_data = match &self.rom_data {
            Some(data) => data.clone(),
            None => return,
        };

        self.patch_status = PatchStatus::Patching;
        let result = patcher::patch_rom(&mut rom_data);
        if result.is_err() {
            self.patch_status = PatchStatus::Error("Patch verification failed".to_string());
        } else {
            self.patch_status = PatchStatus::Success;
        }

        self.rom_data = Some(rom_data);
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
            // Try all supported file managers in order of popularity
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
