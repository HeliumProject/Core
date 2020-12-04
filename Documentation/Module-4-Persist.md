<a href="http://heliumproject.github.io/">![Helium Core](https://raw.github.com/HeliumProject/Core/master/Documentation/Helium.png)</a>

Persist is an implementation of object persistence built atop the [Reflect](https://github.com/HeliumProject/Reflect) C++ reflection library.  It currently implements the following formats:
* [BSON](http://bsonspec.org/) using [mongo-c](https://github.com/mongodb/mongo-c-driver)
* [JSON](http://json.com/) using [rapidjson](http://code.google.com/p/rapidjson/)
* [MessagePack](http://msgpack.org/) using [Foundation](https://github.com/HeliumProject/Foundation)

Persist completely automates the serialization of an object to and from a flat byte buffer or file.  C++ reflection information provides the necessary metadata about the member variable layout of a class of object.  In the general case, objects will be factory allocated when reading a file or buffer, but the user can also specify an existing object to read state into.

## Design

Persist is written to easily facilitate reading and writing data between both files and in-memory buffers.

## Implementation

Note there are some custom types defined in the BSON implementation, as well as custom read/write code for native type support in the format.
