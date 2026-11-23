# M044 Architecture

M018 provides addressed device; M019 parses interface and endpoint descriptors; M020 identifies class; M044 selects MSC BOT interface and prepares protocol packets. M022 currently exposes interrupt-IN transfer APIs only, so bulk endpoint transfer is not available. M044 deliberately does not claim BOT transport or USB block-device support.
