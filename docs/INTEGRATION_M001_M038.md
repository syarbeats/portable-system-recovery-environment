# PSRE Integration M001–M038

Cumulative Build 3.3 retains M001–M037 and adds M038. M038 associates M036 applications with M034 windows and provides independent widget containers. M035 initializes M038, creates sample containers when launching built-in applications, routes input to the focused window's container, renders widgets, and reconciles stopped applications.

M037 remains available for legacy global widgets. New application GUI code should use M038 to keep widget IDs and state scoped to its own window.
