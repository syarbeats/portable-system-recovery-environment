# M053 Integration

Build includes `kernel/src/m053/hub_driver.c`. Main initializes M053 after M052. Serial CLI commands: `m053-info`, `m053-probe`, `m053-scan`. `m053-probe` targets the currently configured M020 Hub and registers it as M052 hub ID 1 at root topology. Use only when the active M018/M020 device is a Hub.
