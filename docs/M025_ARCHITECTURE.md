# M025 Architecture

M025 is the consumer of M022 and the coordinator of M023/M024. Initialization prefers a boot keyboard. If none is available, it falls back to a boot mouse. The chosen endpoint is configured once through M022.

Each `m025-poll` performs one bounded Interrupt IN transfer. Keyboard reports are compared against the previous modifier/key state to generate transitions; mouse reports generate relative movement and button transitions. Events are appended to a 64-entry static FIFO. `m025-next` removes the oldest event.

The queue is single-threaded and polled. No locking is provided because the current PSRE input path runs synchronously in the CLI.
