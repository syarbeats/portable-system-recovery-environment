# M046 Test Plan
- Verify BOT initialization requires M044 transport_ready.
- Verify INQUIRY CBW/data/CSW sequence with mocked Bulk transfers.
- Verify returned data length and byte counters.
- Verify reset recovery is reported unavailable rather than falsely succeeding.
- Hardware follow-up: USB MSC flash drive, successful INQUIRY/TUR/READ CAPACITY/READ(10), CSW residue and error injection.
