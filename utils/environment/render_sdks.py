"""Prepare non-Git runtimes after checking selected SDK submodules."""
from utils.environment.sdks import check_sdks, sdk_ready
from utils.environment import streamline
from utils.build import fidelityfx_vulkan


def render_sdks_ready(options=None, backends=()):
    if options is None or not options.enabled:
        return True
    if not all(sdk_ready(name) for name in options.sdk_names(backends)):
        return False
    if options.streamline and not streamline.runtime_ready():
        return False
    if (options.fsr_sr or options.fsr_fg) and 'vulkan' in backends and not fidelityfx_vulkan.runtime_ready():
        return False
    return True


def ensure_render_sdks(options=None, backends=(), generator='Visual Studio 17 2022'):
    if options is None or not options.enabled:
        return
    options.validate('win', backends)
    print('\nChecking selected SR/FG SDKs...')
    check_sdks(options.sdk_names(backends))
    if options.streamline:
        streamline.ensure_runtime()
    if (options.fsr_sr or options.fsr_fg) and 'vulkan' in backends:
        fidelityfx_vulkan.ensure_runtime(generator=generator)
