---
id: how-to
title: How to use GraphAr PySpark package
sidebar_position: 1
---


## GraphAr PySpark

``graphar_pyspark`` is implemented as bindings to GraphAr spark scala
library. You should have ``graphar-0.1.0-SNAPSHOT.jar`` in your
Apache Spark JVM classpath. Otherwise you will get an exception. To
add it specify ``config("spark.jars", "path-to-graphar-jar")`` when
you create a SparkSession:

```python
from pyspark.sql import SparkSession

spark = (
    SparkSession
    .builder
    .master("local[1]")
    .appName("graphar-local-tests")
    .config("spark.jars", "../../spark/graphar/target/graphar-0.1.0-SNAPSHOT.jar")
    .config("spark.log.level", "INFO")
    .getOrCreate()
)
```

## GraphAr PySpark initialize

PySpark bindings are heavily relying on JVM-calls via ``py4j``. To
initiate all the necessary things for it just call
``graphar_pyspark.initialize()``:

```python
from graphar_pyspark import initialize

initialize(spark)
```

## GraphAr objects

Now you can import, create and modify all the classes you can
call from [scala API of GraphAr](https://graphar.apache.org/docs/spark/).
For simplify using of graphar from python constants, like GAR-types,
supported file-types, etc. are placed in ``graphar_pyspark.enums``.

```python
from graphar_pyspark.info import Property, PropertyGroup, AdjList, AdjListType, VertexInfo, EdgeInfo, GraphInfo
from graphar_pyspark.enums import GarType, FileType
```

Main objects of GraphAr are the following:

- GraphInfo
- VertexInfo
- EdgeInfo

You can check [Scala library documentation](../spark/spark.md)
for the more detailed information.

## Creating objects in graphar_pyspark

GraphAr PySpark package provide two main ways how to initiate
objects, like ``GraphInfo``:

- ``from_python(**args)`` when you create an object based on
   python-arguments
- ``from_scala(jvm_ref)`` when you create an object from the
   corresponded JVM-object (``py4j.java_gateway.JavaObject``)

```python
help(Property.from_python)

Help on method from_python in module graphar_pyspark.info:

from_python(name: 'str', data_type: 'GarType', is_primary: 'bool') -> 'PropertyType' method of builtins.type instance
       Create an instance of the Class from Python arguments.
       
       :param name: property name
       :param data_type: property data type
       :param is_primary: flag that property is primary
       :returns: instance of Python Class.
```

```python
python_property = Property.from_python(name="my_property", data_type=GarType.INT64, is_primary=False)
print(type(python_property))

<class 'graphar_pyspark.info.Property'>
```

You can always get a reference to the corresponding JVM object. For
example, if you want to use it in your own code and need a direct link
to the underlying instance of Scala Class, you can just call
``to_scala()`` method:

```python
scala_obj = python_property.to_scala()
print(type(scala_obj))

<class 'py4j.java_gateway.JavaObject'>
```

As we already mentioned, you can initialize an instance of the Python
class from the JVM object:

```python
help(Property.from_scala)

Help on method from_scala in module graphar_pyspark.info:

   from_scala(jvm_obj: 'JavaObject') -> 'PropertyType' method of builtins.type instance
       Create an instance of the Class from the corresponding JVM object.
       
       :param jvm_obj: scala object in JVM.
       :returns: instance of Python Class.
```

```python
python_property = Property.from_scala(scala_obj)
```

Each public property and method of the Scala API is provided in
python, but in a pythonic-naming convention. For example, in Scala,
``Property`` has the following fields:

- name
- data_type
- is_primary

For each of such a field in Scala API there is a getter and setter
methods. You can call them from the Python too:

```python
python_property.get_name()

'my_property'
```

You can also modify fields, but be careful: when you modify field of
instance of the Python class, you modify the underlying Scala Object
at the same moment!

```python
new_name = "my_renamed_property"
python_property.set_name(new_name)
python_property.get_name()

'my_renamed_property'
```

## Loading Info objects from YAML

But manual creating of objects is not a primary way of using GraphAr
PySpark. ``GraphInfo``, ``VertexInfo`` and ``EdgeInfo`` can be also
initialized by reading from YAML-files:

```python
modern_graph_v_person = VertexInfo.load_vertex_info("../../testing/modern_graph/person.vertex.yml")
modern_graph_e_person_person = EdgeInfo.load_edge_info("../../testing/modern_graph/person_knows_person.edge.yml")
modern_graph = GraphInfo.load_graph_info("../../testing/modern_graph/modern_graph.graph.yml")
```

After that you can work with such an objects like regular python
objects:

```python
print(modern_graph_v_person.dump())

"
chunk_size: 2
prefix: vertex/person/
property_groups:
  - prefix: id/
    file_type: csv
    properties:
      - is_primary: true
        name: id
        data_type: int64
  - prefix: name_age/
    file_type: csv
    properties:
      - is_primary: false
        name: name
        data_type: string
      - is_primary: false
        name: age
        data_type: int64
label: person
version: gar/v1
"      
```

```python
print(modern_graph_v_person.contain_property("id") is True)
print(modern_graph_v_person.contain_property("bad_id?") is False)
            
True
True
```

Please, refer to Scala API and examples of GraphAr Spark Scala
library to see detailed and business-case oriented examples!
