# M025 — Input Manager & Event Queue

## Objective
Provide a common kernel input interface for keyboard and mouse boot reports.

## Features
- Select one supported HID boot keyboard; if none exists, select one supported HID boot mouse.
- Configure the selected interface's Interrupt IN endpoint through M022.
- Poll one report on demand.
- Convert keyboard report state changes into key-down/key-up events.
- Convert mouse movement and button state changes into mouse events.
- Store events in a fixed 64-entry FIFO queue.
- Count generated, dropped, and transfer-error events.

## CLI
- `m025-start`: initialize the input manager and selected HID endpoint.
- `m025-poll`: poll one report.
- `m025-next`: dequeue and display one event.
- `m025-info`: display input manager status.

## Event types
1. KEY_DOWN
2. KEY_UP
3. MOUSE_MOVE
4. MOUSE_BUTTON_DOWN
5. MOUSE_BUTTON_UP

## Limitations
- One active HID interface/endpoint at a time because M022 currently configures one endpoint per lifecycle.
- Keyboard has boot-protocol 6-key rollover only; mouse has 3-byte boot report only.
- Polling is explicit, not background/scheduler-driven.
- No character translation, key repeat, wheel, GUI cursor, multi-device aggregation, or interrupt-driven input.
