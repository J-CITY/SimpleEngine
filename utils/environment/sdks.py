"""Read-only checks for SDKs supplied by repository submodules."""
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
# Repository path, initial pin, required headers/libraries and runtime binaries.
SDK_NAMES = {
    'dlss': ('3rd/DLSS', '374959484e79a640feaba44c93ac8cfb0a03f5b5',
             ('include/nvsdk_ngx.h', 'include/nvsdk_ngx_helpers.h',
              'lib/Windows_x86_64/x64/nvsdk_ngx_d.lib', 'lib/Windows_x86_64/x64/nvsdk_ngx_d_dbg.lib'),
             ('lib/Windows_x86_64/rel/nvngx_dlss.dll',)),
    'fsr-vulkan': ('3rd/FidelityFX-Vulkan', 'c6efa6bf7f2027b3ec94f28578bb5965eabb9e55',
                   ('ffx-api/include/ffx_api/ffx_upscale.hpp', 'ffx-api/include/ffx_api/ffx_framegeneration.hpp',
                    'ffx-api/include/ffx_api/vk/ffx_api_vk.hpp'),
                   ('PrebuiltSignedDLL/amd_fidelityfx_vk.dll',)),
    'fsr-dx12': ('3rd/FSR-DX12', '60f4ea81909200d8542eca14dccb2628b763a9a3',
                 ('Kits/FidelityFX/api/include/ffx_api.hpp', 'Kits/FidelityFX/upscalers/include/ffx_upscale.hpp',
                  'Kits/FidelityFX/framegeneration/include/ffx_framegeneration.hpp'),
                 tuple('Kits/FidelityFX/signedbin/' + name + '.dll' for name in
                       ('amd_fidelityfx_loader_dx12', 'amd_fidelityfx_upscaler_dx12', 'amd_fidelityfx_framegeneration_dx12'))),
    'streamline': ('3rd/Streamline', '2122257e0fce486f91b385aa63b9a09b0a34b363',
                   tuple('include/' + name for name in
                         ('sl.h', 'sl_dlss.h', 'sl_dlss_g.h', 'sl_reflex.h', 'sl_pcl.h', 'sl_security.h')), ()),
}


def recorded_pin(sdk):
    entry = subprocess.check_output(['git', 'ls-files', '--stage', '--', sdk[0]], cwd=ROOT, text=True).strip()
    if entry and not entry.startswith('160000 '):
        raise RuntimeError(f'{sdk[0]} is tracked as ordinary files, expected a submodule')
    return entry.split()[1] if entry else sdk[1]


def sdk_ready(name):
    relative, _, headers, binaries = SDK_NAMES[name]
    folder = ROOT / relative
    if not (folder / '.git').exists() or not all((folder / path).is_file() for path in headers):
        return False
    expected = recorded_pin(SDK_NAMES[name])
    result = subprocess.run(['git', 'rev-parse', 'HEAD'], cwd=folder, capture_output=True, text=True)
    if result.returncode or result.stdout.strip() != expected:
        return False
    for relative in binaries:
        try:
            with (folder / relative).open('rb') as stream:
                if stream.read(2) != b'MZ':
                    return False
        except OSError:
            return False
    return True


def check_sdks(names):
    for name in names:
        sdk = SDK_NAMES[name]
        if not sdk_ready(name):
            raise RuntimeError(f'{sdk[0]} is incomplete or at the wrong commit. '
                               'Prepare the repository submodules with git submodule update --init, '
                               'and materialize Git LFS files if applicable.')
        print(f'{sdk[0]} is ready at {recorded_pin(sdk)}', flush=True)
