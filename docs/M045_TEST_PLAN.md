# M045 Test Plan

Host tests stub the M015/M018 hardware interfaces and simulate command/transfer completion events.

Cases: valid Bulk OUT and IN endpoint configuration; invalid zero MPS rejection; successful OUT and IN transfer completion; endpoint status direction; aggregate endpoint and transfer counters.

Hardware tests still required: connect a USB MSC device, configure both endpoints, test INQUIRY data stage and OUT CBW stage, exercise short packets/stalls/timeouts, and validate recovery after endpoint errors.
