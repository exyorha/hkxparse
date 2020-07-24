# hkxparse

hkxparse is a library implementing a parser for HKX files used by the Havok
physics engine. It parses both "packfile"-type files (generally used by older
games) and "tagfile"-type files. Note that packfile parsing requires a
matching "packfile layout" file for that Havok version. Currently, the tool
to create these files remains unpublished.

See hkxparse-test for an usage example.

Please note that hkxparse is incomplete and may fail to parse some files or
parse them incorrectly.

# Building

hkxparse may be built using normal CMake procedures, and is generally
intended to included into an outer project as a submodule or by any other
means.

Ruby interpreter is required during build process, but not in runtime.

Please note that hkxparse uses git submodules, which should be retrieved
before building.

# Licensing

hkxparse is licensed under the terms of the MIT license (see LICENSE).

Note that hkxparse also references the MIT-licensed half-precision float
library by Mike Acton as a submodule.