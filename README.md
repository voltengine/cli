# Volt CLI

⌨️ Volt package manager.

Volt CLI is required by both the hub and editor as a project management and build tool.

## Example Usage

```
# Base

# Command help and version info:
volt help [{command}]

# Update CLI to the latest version and download latest Volt:
volt update

# Get latest/specified Volt/package version (versions are Git tags; volt package refers Volt repo):
volt install ({git-url} | {package}) [{version}]

# List installed packages:
volt list

# Describe remote package and show available versions:
volt info {package}

# Search for remote packages by keywords in their names and descriptions:
volt search {keywords}

# Project Actions

# Create a package:
volt init # ask for: name, version, engineVersion, displayName, description, publisher

# Build editor DLLs or generate shipping files:
volt build [{platform}]

# Build and run the editor:
volt edit

# + Future dependency management and package deployment.
```

Packages are stored as:
```
.volt/packages/{id}/{version}/package.json
```