API Reference
==================

.. _cpp-api:

.. default-domain:: cpp

Graph Info
-----------

.. doxygenclass:: graphar::Property
    :members:
    :undoc-members:

.. doxygenclass:: graphar::PropertyGroup
    :members:
    :undoc-members:

.. doxygenfunction:: graphar::CreatePropertyGroup

.. doxygenclass:: graphar::AdjacentList
    :members:
    :undoc-members:

.. doxygenfunction:: graphar::CreateAdjacentList

.. doxygenclass:: graphar::VertexInfo
    :members:
    :undoc-members:

.. doxygenfunction:: graphar::CreateVertexInfo

.. doxygenclass:: graphar::EdgeInfo
    :members:
    :undoc-members:

.. doxygenfunction:: graphar::CreateEdgeInfo

.. doxygenclass:: graphar::GraphInfo
    :members:
    :undoc-members:

.. doxygenfunction:: graphar::CreateGraphInfo

Readers
---------------------

Chunk Info Reader
~~~~~~~~~~~~~~~~~

.. doxygenclass:: graphar::VertexPropertyChunkInfoReader
    :members:
    :undoc-members:

.. doxygenclass:: graphar::AdjListChunkInfoReader
    :members:
    :undoc-members:

.. doxygenclass:: graphar::AdjListPropertyChunkInfoReader
    :members:
    :undoc-members:

Arrow Chunk Reader
~~~~~~~~~~~~~~~~~~

.. doxygenclass:: graphar::VertexPropertyArrowChunkReader
    :members:
    :undoc-members:

.. doxygenclass:: graphar::AdjListArrowChunkReader
    :members:
    :undoc-members:

.. doxygenclass:: graphar::AdjListOffsetArrowChunkReader
    :members:
    :undoc-members:

Vertices Collection
~~~~~~~~~~~~~~~~~~~

.. doxygenclass:: graphar::Vertex
    :members:
    :undoc-members:

.. doxygenclass:: graphar::VertexIter
    :members:
    :undoc-members:

.. doxygenclass:: graphar::VerticesCollection
    :members:
    :undoc-members:

Edges Collection
~~~~~~~~~~~~~~~~~~

.. doxygenclass:: graphar::Edge
    :members:
    :undoc-members:

.. doxygenclass:: graphar::EdgeIter
    :members:
    :undoc-members:

.. doxygenclass:: graphar::EdgesCollection
    :members:
    :undoc-members:

.. doxygenclass:: graphar::OBSEdgeCollection
    :members:
    :undoc-members:

.. doxygenclass:: graphar::OBDEdgesCollection
    :members:
    :undoc-members:

.. doxygenclass:: graphar::UBSEdgesCollection
    :members:
    :undoc-members:

.. doxygenclass:: graphar::UBDEdgesCollection
    :members:
    :undoc-members:

Writer and Builder
---------------------

Chunk Writer
~~~~~~~~~~~~~~~~~

.. doxygenclass:: graphar::VertexPropertyWriter
    :members:
    :undoc-members:

.. doxygenclass:: graphar::EdgeChunkWriter
    :members:
    :undoc-members:

Builder
~~~~~~~~~~~~~~~~~~~

.. doxygenclass:: graphar::builder::Vertex
    :members:
    :undoc-members:

.. doxygenclass:: graphar::builder::Edge
    :members:
    :undoc-members:

.. doxygenclass:: graphar::builder::VerticesBuilder
    :members:
    :undoc-members:

.. doxygenclass:: graphar::builder::EdgesBuilder
    :members:
    :undoc-members:


Types
--------

Id Type
~~~~~~~~~~~~~~~~~~~

.. doxygentypedef:: graphar::IdType

Data Type
~~~~~~~~~~~~~~~~~~~

.. doxygenclass:: graphar::DataType
    :members:
    :undoc-members:

.. doxygenfunction:: graphar::boolean
.. doxygenfunction:: graphar::int32
.. doxygenfunction:: graphar::int64
.. doxygenfunction:: graphar::float32
.. doxygenfunction:: graphar::float64
.. doxygenfunction:: graphar::string
.. doxygenfunction:: graphar::list

File Type
~~~~~~~~~~~~~~~~~~~
.. doxygenenum:: graphar::FileType

Adj List Type
~~~~~~~~~~~~~~~~~~~
.. doxygenenum:: graphar::AdjListType

Validate Level
~~~~~~~~~~~~~~~~~~~
.. doxygenenum:: graphar::ValidateLevel


Utilities
---------

Result and Status
~~~~~~~~~~~~~~~~~~~

.. doxygentypedef:: graphar::Result

.. doxygenclass:: graphar::Status
    :members:
    :undoc-members:

FileSystem
~~~~~~~~~~~~~~~~~~~

.. doxygenclass:: graphar::FileSystem
    :members:
    :undoc-members:

.. doxygenfunction:: graphar::FileSystemFromUriOrPath

Yaml Parser
~~~~~~~~~~~~~~~~~~~

.. doxygenclass:: graphar::Yaml
    :members:
    :undoc-members:

Info Version
~~~~~~~~~~~~~~~~~~~

.. doxygenclass:: graphar::InfoVersion
    :members:
    :undoc-members:

Expression
~~~~~~~~~~~~~~~~~~~

.. doxygenclass:: graphar::Expression
    :members:
    :undoc-members:

.. doxygenclass:: graphar::ExpressionProperty
    :members:
    :undoc-members:

.. doxygenclass:: graphar::ExpressionLiteral
    :members:
    :undoc-members:

.. doxygenclass:: graphar::ExpressionNot
    :members:
    :undoc-members:

.. doxygenclass:: graphar::ExpressionUnaryOp
    :members:
    :undoc-members:

.. doxygenclass:: graphar::ExpressionBinaryOp
    :members:
    :undoc-members:

.. doxygenclass:: graphar::ExpressionEqual
    :members:
    :undoc-members:

.. doxygenclass:: graphar::ExpressionNotEqual 
    :members:
    :undoc-members:

.. doxygenclass:: graphar::ExpressionGreaterThan
    :members:
    :undoc-members:

.. doxygenclass:: graphar::ExpressionGreaterEqual
    :members:
    :undoc-members:


.. doxygenclass:: graphar::ExpressionLessThan
    :members:
    :undoc-members:

.. doxygenclass:: graphar::ExpressionLessEqual 
    :members:
    :undoc-members:

.. doxygenclass:: graphar::ExpressionAnd
    :members:
    :undoc-members:

.. doxygenclass:: graphar::ExpressionOr
    :members:
    :undoc-members:

.. doxygenfunction:: graphar::_Property(const Property&)
.. doxygenfunction:: graphar::_Property(const std::string&)
.. doxygenfunction:: graphar::_Literal
.. doxygenfunction:: graphar::_Not
.. doxygenfunction:: graphar::_Equal
.. doxygenfunction:: graphar::_NotEqual
.. doxygenfunction:: graphar::_GreaterThan
.. doxygenfunction:: graphar::_GreaterEqual
.. doxygenfunction:: graphar::_LessThan
.. doxygenfunction:: graphar::_LessEqual
.. doxygenfunction:: graphar::_And
.. doxygenfunction:: graphar::_Or

