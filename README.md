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

# Describe remote package and show available versions:
volt info {package-id}

# Search for remote packages by keywords:
volt search {keywords}

# Get latest/specified package version (versions are Git tags):
volt install {package-id} [{version}]

# Remove all/specified package version(s) (versions are Git tags):
volt remove {package-id} [{version}]

# Create a package:
volt init # ask for: id, title, publisher, description

# Set a dependency (install if not cached):
volt depend {package-id} [{version}]

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