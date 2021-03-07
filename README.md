# Volt CLI

⌨️ Volt package manager.

Volt CLI is required by both the hub and editor as a project management and build tool.

## Example Usage

```
# Command help and version info:
volt help [{command}]

# Update CLI to the latest version:
volt update

# List installed/available Volt versions:
volt list (local|remote)

# Install latest/specified Volt version:
volt install [{version}]

# Create a package:
volt init # ask for: name, version, engineVersion, displayName, description, publisher

# Build editor DLLs or generate shipping files:
volt build [{platform}]

# Build and run the editor:
volt edit

# Convert package to newer Volt version:
volt convert {version}

# + Future dependency management and package deployment.
```
