set(SPLA_CPP_SOURCES
        sources/SplaData.cpp
        sources/SplaDescriptor.cpp
        sources/SplaExpression.cpp
        sources/SplaExpressionNode.cpp
        sources/SplaFunctionBinary.cpp
        sources/SplaFunctionUnary.cpp
        sources/SplaLibrary.cpp
        sources/SplaMatrix.cpp
        sources/SplaRefCnt.cpp
        sources/SplaScalar.cpp
        sources/SplaType.cpp
        sources/SplaVector.cpp)

set(SPLA_DETAIL_SOURCES
        sources/detail/SplaError.hpp
        sources/detail/SplaLibraryPrivate.cpp
        sources/detail/SplaLibraryPrivate.hpp)

set(SPLA_EXPRESSION_SOURCES
        sources/expression/SplaExpressionManager.cpp
        sources/expression/SplaExpressionManager.hpp
        sources/expression/SplaNodeProcessor.hpp
        )