<a href="http://heliumproject.org/">![Helium Game Engine](https://raw.github.com/HeliumProject/Helium/master/Data/Textures/Helium.png)</a>

Mongo completely automates the serialization of an object to and from a MongoDB database.  The [Reflect](https://github.com/HeliumProject/Reflect) C++ reflection library provides information about the member variable layout of a class of object.  Model objects contain only a single new member variable: id, which follows the _id variable that stores the internal unique id of an object in MongoDB.
Inserts are only valid using a model object with a null id, and Updates are only valid with a model object with a non-null id.

Design
======

This module was written to implement the database modelling design pattern in C++ using reflection information provided by Reflect.  At the moment its focused on correctly serializing C++ classes to and from BSON within the context of a database connection.  Queries and indexes must be implemented outside the scope of this module.

Implementation
==============

Mongo was written to not rely on locks when accessing the database.  Mongo::Database merely tracks which thread is intended to use the database connection, and it will assert if the wrong thread causes the connection to be used.

Location
========
https://github.com/HeliumProject/Persist
