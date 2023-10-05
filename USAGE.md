## Build
By default, ninbox doesn't know where 86Box is installed. To specify the location of 86Box, compile with
```sh
./build.sh --boxpath /location/of/86Box
```
To enable colors, you can specify `--color` option when invoking the build script. However, it ignores `--boxpath`, so if you want to specify both parameters, use the `--all` option.

## Usage
#### Load a machine config
```sh
# By name (string)
## Command:
$ ninbox --load-name [name]
## Example:
$ ninbox --load-name 'Windows 98'

# By position (integer)
## Command:
$ ninbox --load-num [pos]
## Example:
$ ninbox --load-num 2

# Passing multiple args
## Example:
$ ninbox --load-name 'Windows 98' 'NT 3'
$ ninbox --load-num 2 5
```

#### Edit a machine config
```sh
# By name (string)
## Command:
$ ninbox --edit-name [name]
## Example:
$ ninbox --edit-name 'Windows 98'

# By position (integer)
## Command:
$ ninbox --edit-num [pos]
## Example:
$ ninbox --edit-num 2

# Passing multiple args
## Example:
$ ninbox --edit-name 'Windows 98' 'NT 3'
$ ninbox --edit-num 2 5
```

#### Delete a machine config
```sh
# By name (string)
## Command:
$ ninbox --del-name [name]
## Example:
$ ninbox --del-name 'Windows 98'

# By position (integer)
## Command:
$ ninbox --del-num [pos]
## Example:
$ ninbox --del-num 2

# Passing multiple args
## Example:
$ ninbox --del-name 'Windows 98' 'NT 3'
$ ninbox --del-num 2 5
```

#### View a machine config
```sh
# By name (string)
## Command:
$ ninbox --view-name [name]
## Example:
$ ninbox --view-name 'Windows 98'

# By position (integer)
## Command:
$ ninbox --view-num [pos]
## Example:
$ ninbox --view-num 2

# Passing multiple args
## Example:
$ ninbox --view-name 'Windows 98' 'NT 3'
$ ninbox --view-num 2 5
```

#### List all available config
```sh
## Command:
$ ninbox --list
```

#### Set a ROM directory
```sh
## Command:
$ ninbox --rom-path [path] [name]
## Example:
$ ninbox --rom-path . './Windows 98'
```
Note that setting a ROM directory isn't permanent and will be forgotten once you close 86Box.

#### Set a root directory
```sh
## Command:
$ ninbox --root-path [path] [name]
## Example:
$ ninbox --root-path . './Windows 98'
```
Note that setting a ROM directory isn't permanent and will be forgotten once you close 86Box.

#### Open Settings
```sh
## Command:
$ ninbox --settings 'Windows 98'
```
