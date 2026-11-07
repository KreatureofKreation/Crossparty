# Native controller spike (for 0.2.0)

The question: can Crossparty work with a controller out of the box, without
the user remapping Steam Input, and without ever acting on controller input
while a game has focus? Run on 2026-11-07 against the released 0.1.0
Flatpak, on the desktop, with an 8BitDo Ultimate Wireless controller through
its USB receiver. Every test ran inside the app's own Flatpak sandbox, with
the extra permissions added on the command line only (`flatpak run
--device=…`), never in the manifest.

**Verdict:** native support is possible and safe, but not through the
browser's Gamepad API. Crossparty should read the controller itself with
SDL3, only while its window has focus, and turn it into keys for the page.

| Test | Result | Notes |
|---|---|---|
| Flatpak can grant controllers narrowly | Pass | Flatpak 1.18.4 supports `--device=input` (only `/dev/input`, not every device). |
| SDL is in the runtime | Pass | org.kde.Platform 6.11 ships SDL3 3.2 and SDL2; nothing new to bundle. |
| The page's Gamepad API, with `--device=input` only | Fail | `navigator.getGamepads()` was always empty. The sandbox could read the controller (`/dev/input/event17`), but has no `/run/udev`, which Chromium uses to find controllers. |
| The page's Gamepad API, plus `--filesystem=/run/udev:ro` | Pass | Chromium saw a "standard" gamepad, and play.xbox.com navigated itself with the D-pad and A: it is built for consoles. |
| The page only gets input while focused | **Fail** | With Crossparty not focused, and with it minimised (the page in the background holder), the page still read every press: D-pad (buttons 12, 13, 14) and A (button 0). Microsoft's page chose to ignore them, so nothing visible happened, but the input reached it. |
| SDL3 sees the controller without `/run/udev` | Pass | Inside the sandbox with `--device=input` only, SDL3 found the controller by name and reported A, B and all four D-pad directions in order. |
| The page navigates with keys | Pass | Arrow keys and Enter drive play.xbox.com once the page itself has keyboard focus; before that they went to Crossparty's own controls. |

## What this means

The Gamepad API route is the easiest, and it would make the whole page work
with a controller. It is rejected because Chromium gives the page every
button press even while the user is in a game. Whether a stray A press in a
game ever reaches a button in the party (Leave party, for example) would then
depend on Microsoft's page continuing to ignore unfocused input, and
Microsoft's code would be able to see everything the user presses in their
games. The design rule is that Crossparty acts on controller input only while
it has focus, and that rule has to be enforced by Crossparty, not by the page.

The proposed design:

- Grant `--device=input` and **not** `/run/udev`. Without udev, Chromium finds
  no controllers, so the page never receives gamepad input at all.
- The C++ core reads controllers with SDL3. While Crossparty's window is
  focused, it turns the D-pad and left stick into arrow keys, A into Enter
  and B into Escape, and delivers them to the focused item: the handheld bar,
  or the page view, which is given keyboard focus first. While the window is
  not focused, SDL events are dropped before they become anything.
- The Guide button stays Steam's. Opening the overlay over a game keeps using
  the Steam chord and the hotkey (README, Controller setup).

## Open questions for the build

1. **Game Mode.** In Game Mode, Steam Input usually gives a non-Steam app a
   virtual Xbox controller ("Steam Virtual Gamepad") and may hide the
   physical one. SDL is expected to see the virtual one, but this needs the
   real-handheld test, or nested Gamescope with Steam Input running.
2. **Focus under Gamescope.** Crossparty uses its window's focus to gate
   input. Gamescope gives focus to the app it shows, so this should hold, but
   it has to be checked.
3. **Duplicate input on the desktop.** If Steam's desktop layout is also
   sending keys for the same press, the page would get it twice. Crossparty
   may need to ignore controllers while Steam Input is translating them, or
   tell the user to pick one.
4. **The extra permission.** `--device=input` lets the app open keyboards and
   mice too, not just controllers. SDL only opens gamepads, and Flathub
   accepts this permission for controller support, but it needs its own SEC
   entry and accepted risk: the permission cannot enforce the focus rule;
   only Crossparty's code does.

## Sessions

### 2026-11-07

- Checked Flatpak and runtime support (above).
- Ran 0.1.0 with `--device=input` and the debug port on 127.0.0.1, and
  polled `navigator.getGamepads()` over the debug protocol every 2 seconds:
  empty. The sandbox could read the device; `/run/udev` was missing.
- Added `--filesystem=/run/udev:ro`: the page saw the controller and
  navigated with it. Pressed the D-pad and A with Crossparty unfocused, and
  again minimised: the page read every press. Nothing happened on screen.
- Built a 40-line SDL3 test with the KDE SDK and ran it in the app's sandbox
  with `--device=input` only: the controller and every press were seen.
- Arrow keys and Enter drive the page once it has keyboard focus.
- Closed the debug port afterwards and restarted the app normally.
