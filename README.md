# Volt CLI

⌨️ Volt package manager.

Volt CLI is required by both the hub and editor as a project management and build tool.

## Development


```
# Set environment variable `VOLT_PATH` to `./.volt/`.

# Install Conan dependencies:
conan install -s build_type=Release -if build .

# Configure and build using CMake:
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target volt_cli --config Release
```

## Example Usage

```
# Command help and version info:
volt help [{command}]

# List installed packages:
volt list

# Describe current/remote package and show available versions:
volt info [[{scope}/]{name}]

# Search for remote packages by keywords:
volt search {keywords}

# Generate 'package.json':
volt init # asks for id, description, etc.

# Check for dependency collisions and download missing components.
# If a package ID is provided, new dependency will be added to ./package.json, then command continues on as normal.
# Version is required to be in format: `major.minor`.
volt install [{id} [{major}.{minor}]]

# Remove dependency from `package.json`:
# volt uninstall {id}

# Authenticates user to selected archive.
# - Requests client ID from archive.
# - Receives a token from the user via GitHub device flow.
# - Saves to `./volt/config.json` for future use.
volt auth

# Uploads 'package.json' to selected archive using token from '.volt/config.json'.
# Can invoke `volt auth` if not authenticated.
volt publish

# Removes package or its single version from selected archive.
volt unpublish {id} [{version}]

# Delete all installed package versions or a single one specified:
# volt remove {id} [{version}]

# Build and run the editor:
volt edit

# Build the editor and execute headless build:
volt build [{platform}]

# + Future dependency management and package deployment.
```

Packages are stored as:
```
.volt/packages/{id}/{version}/package.json
```