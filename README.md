# AXE11:
A Proof-of-Concept libX11 Shim for Gamemaker Games to run under Box86 with GL4ES (and the necessary set of hacks on top of it).

### Dependencies:
-----
To actually run said GameMaker games, you'll also need the experimental versions of [Box86](https://github.com/JohnnyonFlame/box86) and [GL4ES](https://github.com/JohnnyonFlame/gl4es) provided, and for most games, a way to [remap your joystick](https://github.com/lualiliu/RG351P_virtual-gamepad).

As an example, this is the script I'm using to test _Nuclear Throne_ (on 351ELEC):

```bash
#!/bin/bash

export LIBGL_ES=3
export LIBGL_FBONOALPHA=1
export LIBGL_ALPHAHACK=1
export LIBGL_FASTMATH=1
export LIBGL_GL=21
export LIBGL_FB=4
export BOX86_ALLOWMISSINGLIBS=1
export BOX86_LOG=0
export LD_LIBRARY_PATH=/roms/ports/box86/lib:/usr/lib32:/storage/axe11
export BOX86_LD_LIBRARY_PATH=/roms/ports/box86/lib:/usr/lib32/:./:lib/:lib32/:x86/:/storage/axe11
export BOX86_DYNAREC=1

~/rg351p-js2xbox &
/roms/ports/box86/box86 runner
killall -9 rg351p-js2xbox
```

### Building and Deploying [example]:
-----
```bash
make -j$(($(nproc)+1))
scp build/libs/* root@351elec:~/axe11/ # On 351Elec
scp build/libs/* ark@rg351p:~/axe11/ # On ArkOS
```

### Limitations:
-----
Most important: Currently no support. Don't (yet) open issues if you're not contributing solutions or code.

This is a very simple, and very incomplete set of hacks, shims and stubs to run X11 Gamemaker titles on RK3326 handhelds such as the Anbernic RG351P. This isn't meant as a final solution, rather a Proof-of-Concept piece working towards a more complete solution using pure GBM.

Currently you need `export BOX86_ALLOWMISSINGLIBS=1` in order to run games due to a few missing libraries. This is known and shouldn't stop you from running it. If you require any additional libraries or symbols, PRs are welcome even if just to stub a couple new functions or add a few empty libraries as long as the PR states this clearly.

X11 Error reporting is _not_ implemented. All the `return BadWindow;` and similar only return such as means to have something stick out during debugging, and are not _actual_ error handling.

SDL1.2 and 2.0 are known not to work, so this can't fix issues with Chowdren-based games such as _Freedom Planet_ or _Baba Is You_. This probably won't be fixed by this in the near future. Look for alternative solutions instead.

### License:
-----
This is free software. The source files in this repository are released under the [Modified BSD License](LICENSE.md), see the license file for more information.