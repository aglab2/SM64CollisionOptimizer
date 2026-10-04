#[path = "../build/embedded_data.rs"]
mod embedded_data;

mod checksum;
mod patcher;

use egui::{Color32, Panel, RichText, ScrollArea, Ui, Vec2};
use std::{path::PathBuf};
use anyhow::{Context, Result};

#[derive(Default)]
struct App {
    patched_path: Option<PathBuf>,
    patch_status: PatchStatus,
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
            // TODO: Planned configs
            ui.label("Hi!");
        });

        Panel::bottom("bottom_panel").show(ui, |ui| {
            match &self.patch_status {
                PatchStatus::Idle => {
                    ui.label("Ready. Open a ROM file to begin.");
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

        let patched_rom = self.patch_rom(rom)
                              .with_context(|| "Failed to patch ROM")?;

        let out_path = path.with_file_name(format!(
            "{}_patched.z64",
            path.file_stem().unwrap().to_string_lossy()
        ));

        std::fs::write(&out_path, patched_rom).with_context(|| format!("Failed to write patched ROM {}", out_path.to_string_lossy()))?;
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
