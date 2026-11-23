# M046 Architecture
M046 is a synchronous BOT state machine layered over M044 and M045:
CBW OUT -> optional DATA IN/OUT -> CSW IN.
The CBW is serialized explicitly to avoid relying on host CPU endianness or packed-structure memory layout. The CSW is validated by M044 against the command tag and expected transfer length.
Transport failures and malformed CSW move the state to NEEDS_RESET. SCSI command failure is distinct from BOT transport/protocol failure.
