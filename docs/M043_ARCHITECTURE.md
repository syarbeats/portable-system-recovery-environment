# M043 Architecture

`m043/process.c` owns the single active job and a 16-entry circular history. M008 remains the source of file staging and integrity metadata. M009 owns the queue pool and the new cooperative export cursor (`file`, `offset`, completed bytes). M043 calls M009 in bounded steps and copies progress into its public status structure.

Queue export starts by printing the M009 manifest, then the export header. Each tick emits at most the requested payload budget, defaulted by M043 to 256 bytes. Each file is framed by BEGIN/D/END/PROGRESS records. The terminal `PSRE-EXPORT-COMPLETE` marker is emitted only after the last file.

M041 Recovery Manager adds Pause/Resume and Retry controls. Its existing Export Queue action starts the M043 export job. The M035 `m035-poll` path advances one job tick. CLI users can also drive jobs manually using `m043-tick`.
