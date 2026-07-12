import sys
import re

def patch_textdecoder_guard(js_path: str) -> None:
    """
    Post-build patch for Emscripten output.

    Emscripten's generated UTF8ArrayToString() calls TextDecoder.decode()
    for strings longer than 16 bytes when the module is compiled with
    ALLOW_MEMORY_GROWTH. TextDecoder.decode() rejects views backed by a
    resizable ArrayBuffer per the WHATWG encoding spec, which crashes any
    string marshalling (e.g. stat64 path decoding) once memory growth is
    enabled. This patch forces the TextDecoder fast path to always be
    skipped, falling back to the manual decode loop unconditionally.

    NOTE: this depends on Emscripten's current generated code shape and
    may need updating if the toolchain version changes.
    """
    with open(js_path, "r", encoding="utf-8") as f:
        content = f.read()

    pattern = r"endPtr - idx > 16 && heapOrArray\.buffer && UTF8Decoder"
    replacement = "false"

    patched_content, count = re.subn(pattern, replacement, content)

    if count == 0:
        sys.exit(
            f"ERROR: TextDecoder patch pattern not found in {js_path}. "
            "Emscripten's generated code may have changed shape - "
            "update the pattern in patch_textdecoder_guard()."
        )

    with open(js_path, "w", encoding="utf-8") as f:
        f.write(patched_content)

    print(f"Patched TextDecoder guard in {js_path} ({count} occurrence(s))")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        sys.exit("Usage: patch_textdecoder.py <path-to-generated-js>")
    patch_textdecoder_guard(sys.argv[1])