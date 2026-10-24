use std::path::PathBuf;
use std::sync::mpsc::{Receiver, Sender, channel};
use rfd::FileDialog;

pub struct DialogThread {
    sender: Sender<ThreadMessage>,
    pending: Option<Receiver<Option<PathBuf>>>,
    thread: Option<std::thread::JoinHandle<()>>,
}

// TODO: will i really want more requests?
#[derive(Debug)]
pub enum DialogRequest {
    OpenRom,
}

enum ThreadMessage {
    Request((DialogRequest, Sender<Option<PathBuf>>)),
    Shutdown,
}

impl DialogThread {
    pub fn new() -> Self {
        let (tx, rx) = channel::<ThreadMessage>();
        let thread = std::thread::Builder::new()
            .name("file-dialog".to_string())
            .spawn(move || {
                loop {
                    match rx.recv() {
                        Ok(ThreadMessage::Request((request, resp_tx))) => {
                            match request {
                                DialogRequest::OpenRom => {
                                    let path = FileDialog::new()
                                        .add_filter("N64 ROM", &["z64"])
                                        .add_filter("All files", &["*"])
                                        .pick_file();
                                    let _ = resp_tx.send(path);
                                }
                            }
                        }
                        Ok(ThreadMessage::Shutdown) | Err(_) => break,
                    }
                }
            })
            .expect("Failed to spawn file dialog thread");

        DialogThread {
            sender: tx,
            pending: None,
            thread: Some(thread),
        }
    }

    pub fn request(&mut self, req: DialogRequest) -> bool {
        if self.pending.is_some() {
            return false; // Dialog already active
        }
        let (resp_tx, resp_rx) = channel();
        let _ = self.sender.send(ThreadMessage::Request((req, resp_tx)));
        self.pending = Some(resp_rx);
        true
    }

    pub fn is_pending(&self) -> bool {
        self.pending.is_some()
    }

    pub fn try_recv(&mut self) -> Option<Option<PathBuf>> {
        let Some(rx) = &self.pending else {
            return None;
        };

        match rx.try_recv() {
            Ok(path) => {
                self.pending = None;
                Some(path)
            }
            Err(std::sync::mpsc::TryRecvError::Empty) => None,
            Err(std::sync::mpsc::TryRecvError::Disconnected) => {
                self.pending = None;
                None
            }
        }
    }
}

impl Default for DialogThread {
    fn default() -> Self {
        Self::new()
    }
}

impl Drop for DialogThread {
    fn drop(&mut self) {
        let _ = self.sender.send(ThreadMessage::Shutdown);
        if let Some(thread) = self.thread.take() {
            let _ = thread.join();
        }
    }
}
