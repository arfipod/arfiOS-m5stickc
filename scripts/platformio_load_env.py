Import("env")

from pathlib import Path


def parse_dotenv(path):
    values = {}
    if not path.exists():
        return values

    for raw_line in path.read_text(encoding="utf-8").splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue

        key, value = line.split("=", 1)
        key = key.strip()
        value = value.strip()

        if (value.startswith('"') and value.endswith('"')) or (value.startswith("'") and value.endswith("'")):
            value = value[1:-1]

        values[key] = value

    return values


def escape_define_string(value):
    return value.replace("\\", "\\\\").replace('"', '\\"')


project_dir = Path(env.subst("$PROJECT_DIR"))
local_env = parse_dotenv(project_dir / ".env")

defines = []
for name in ("ARFI_WIFI_SSID", "ARFI_WIFI_PASSWORD", "ARFI_REST_URL"):
    value = local_env.get(name)
    if value:
        defines.append((name, '\\"{}\\"'.format(escape_define_string(value))))

if defines:
    env.Append(CPPDEFINES=defines)
