How to use GraphAr PySpark package
==================================

.. container:: cell markdown
   :name: b23d0681-da6d-4759-9d62-08d9376712ef

   .. rubric:: GraphAr PySpark
      :name: graphar-pyspark

   ``graphar_pyspark`` is implemented as bindings to GraphAr spark scala
   library. You should have ``graphar-0.1.0-SNAPSHOT.jar`` in your
   Apache Spark JVM classpath. Otherwise you will get an exception. To
   add it spceify ``config("spark.jars", "path-to-graphar-jar")`` when
   you create a SparkSession:

.. container:: cell code
   :name: 40fa9a16-66b7-44d7-8aff-dd84fed0303a

   .. code:: python

      from pyspark.sql import SparkSession

      spark = (
          SparkSession
          .builder
          .master("local[1]")
          .appName("graphar-local-tests")
          .config("spark.jars", "../../spark/target/graphar-0.1.0-SNAPSHOT.jar")
          .config("spark.log.level", "INFO")
          .getOrCreate()
      )

.. container:: cell markdown
   :name: 1e40491b-9395-469c-bc30-ac4378d11265

   .. rubric:: GraphAr PySpark initialize
      :name: graphar-pyspark-initialize

   PySpark bindings are heavily relying on JVM-calls via ``py4j``. To
   initiate all the neccessary things for it just call
   ``graphar_pyspark.initialize()``:

.. container:: cell code
   :name: a1ff3f35-2a5a-4111-a296-b678b318b4dd

   .. code:: python

      from graphar_pyspark import initialize

      initialize(spark)

.. container:: cell markdown
   :name: 180b35c8-c0aa-4c6c-abc0-ffbf2ea1d833

   .. rubric:: GraphAr objects
      :name: graphar-objects

   Now you can import, create and modify all the classes you can work
   call from `scala API of
   GraphAr <https://alibaba.github.io/GraphAr/spark/reference/index.html>`__.
   For simplify using of graphar from python constants, like GAR-types,
   supported file-types, etc. are placed in ``graphar_pyspark.enums``.

.. container:: cell code
   :name: 85e186a4-0c44-450b-ac9d-d8624bb3d1d1

   .. code:: python

      from graphar_pyspark.info import Property, PropertyGroup, AdjList, AdjListType, VertexInfo, EdgeInfo, GraphInfo
      from graphar_pyspark.enums import GarType, FileType

.. container:: cell markdown
   :name: 4b0aad82-df2d-47f9-9799-89b45fe61519

   Main objects of GraphAr are the following:

   -  GraphInfo
   -  VertexInfo
   -  EdgeInfo

   You can check `Scala library
   documentation <https://alibaba.github.io/GraphAr/spark/spark-lib.html#information-classes>`__
   for the more detailed information.

.. container:: cell markdown
   :name: 71ac9d59-521c-41bf-a951-b5d08768096e

   .. rubric:: Creating objects in graphar_pyspark
      :name: creating-objects-in-graphar_pyspark

   GraphAr PySpark package provide two main ways how to initiate
   objects, like ``GraphInfo``:

   #. ``from_python(**args)`` when you create an object based on
      python-arguments
   #. ``from_scala(jvm_ref)`` when you create an object from the
      corresponded JVM-object (``py4j.java_gateway.JavaObject``)

.. container:: cell code
   :name: 560cec49-bb31-4ae5-86aa-f9b24642c283

   .. code:: python

      help(Property.from_python)

   .. container:: output stream stdout

      ::

         Help on method from_python in module graphar_pyspark.info:

         from_python(name: 'str', data_type: 'GarType', is_primary: 'bool') -> 'PropertyType' method of builtins.type instance
             Create an instance of the Class from Python arguments.
             
             :param name: property name
             :param data_type: property data type
             :param is_primary: flag that property is primary
             :returns: instance of Python Class.

.. container:: cell code
   :name: 809301c2-89f3-4ea3-9afd-9154be317972

   .. code:: python

      python_property = Property.from_python(name="my_property", data_type=GarType.INT64, is_primary=False)
      print(type(python_property))

   .. container:: output stream stdout

      ::

         <class 'graphar_pyspark.info.Property'>

.. container:: cell markdown
   :name: 45f0f74d-9568-467f-809a-832f80d5afc6

   You can always get a reference to the corresponding JVM object. For
   example, you want to use it in your own code and need a direct link
   to the underlaying instance of Scala Class, you can just call
   ``to_scala()`` method:

.. container:: cell code
   :name: 9c29c329-76ad-4908-84b6-06e004963ae5

   .. code:: python

      scala_obj = python_property.to_scala()
      print(type(scala_obj))

   .. container:: output stream stdout

      ::

         <class 'py4j.java_gateway.JavaObject'>

.. container:: cell markdown
   :name: 0703a9e0-a48a-4380-8ea6-383cc8164650

   As we already mentioned, you can initialize an instance of the Python
   class from the JVM object:

.. container:: cell code
   :name: 25d243a3-645c-4777-b54e-9175b0685c6f

   .. code:: python

      help(Property.from_scala)

   .. container:: output stream stdout

      ::

         Help on method from_scala in module graphar_pyspark.info:

         from_scala(jvm_obj: 'JavaObject') -> 'PropertyType' method of builtins.type instance
             Create an instance of the Class from the corresponding JVM object.
             
             :param jvm_obj: scala object in JVM.
             :returns: instance of Python Class.

.. container:: cell code
   :name: fbea761b-a843-4225-a589-c66f98d7799c

   .. code:: python

      python_property = Property.from_scala(scala_obj)

.. container:: cell markdown
   :name: 7c54a9b6-29f1-4a57-aa14-30679613b128

   Each public property and method of the Scala API is provided in
   python, but in a pythonic-naming convention. For example, in Scala,
   ``Property`` has the following fields:

   -  name
   -  data_type
   -  is_primary

   For each of such a field in Scala API there is a getter and setter
   methods. You can call them from the Python too:

.. container:: cell code
   :name: ec90236a-cc39-42bc-a5d1-2f57db3a3d8b

   .. code:: python

      python_property.get_name()

   .. container:: output execute_result

      ::

         'my_property'

.. container:: cell markdown
   :name: 81f26098-7eb8-4df5-9764-1b2710f8198c

   You can also modify fields, but be careful: when you modify field of
   instance of the Python class, you modify the underlaying Scala Object
   in the same moment!

.. container:: cell code
   :name: ea88b175-ed5f-4fe3-8753-84916b52c7f2

   .. code:: python

      new_name = "my_renamed_property"
      python_property.set_name(new_name)
      python_property.get_name()

   .. container:: output execute_result

      ::

         'my_renamed_property'

.. container:: cell markdown
   :name: 3b013f4b-ffd6-4de0-9587-d7273cb9c90c

   .. rubric:: Loading Info objects from YAML
      :name: loading-info-objects-from-yaml

   But manual creating of objects is not a primary way of using GraphAr
   PySpark. ``GraphInfo``, ``VertexInfo`` and ``EdgeInfo`` can be also
   initialized by reading from YAML-files:

.. container:: cell code
   :name: 3a6dafea-346f-4905-84c1-6c5eda86bba4

   .. code:: python

      modern_graph_v_person = VertexInfo.load_vertex_info("../../testing/modern_graph/person.vertex.yml")
      modern_graph_e_person_person = EdgeInfo.load_edge_info("../../testing/modern_graph/person_knows_person.edge.yml")
      modern_graph = GraphInfo.load_graph_info("../../testing/modern_graph/modern_graph.graph.yml")

.. container:: cell markdown
   :name: 23c145ba-c4ee-43f0-a8d5-62ffebf1ebf3

   After that you can work with such an objects like regular python
   objects:

.. container:: cell code
   :name: 87e99095-0a5f-4e84-9e0a-35f97e1bf9f5

   .. code:: python

      print(modern_graph_v_person.dump())

   .. container:: output stream stdout

      ::

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

.. container:: cell code
   :name: b9d44691-d07d-43d5-833c-0ede2d9b99d9

   .. code:: python

      print(modern_graph_v_person.contain_property("id") is True)
      print(modern_graph_v_person.contain_property("bad_id?") is False)

   .. container:: output stream stdout

      ::

         True
         True

.. container:: cell markdown
   :name: cc992785-eabc-44c2-a63a-b5eebc9af996

   Please, refer to Scala API and examples of GraphAr Spark Scala
   library to see detailed and business-case oriented examples!

.. container:: cell code
   :name: c8fbdbb6-6d48-47fa-b69e-8022c8d8f5a1

   .. code:: python
