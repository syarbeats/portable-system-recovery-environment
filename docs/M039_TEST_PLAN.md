# M039 Test Plan

1. Initialize M038 and M039, register a running app/window, create a container.
2. Add mixed button and checkbox widgets.
3. Apply vertical layout and verify x/y/stretch width.
4. Apply grid layout and verify row/column placement.
5. Bring a widget to front and redraw only the target container.
6. Shrink the window client area and verify layout rejects an overflowed placement.
7. Run M032–M039 regression host tests and freestanding compilation.
