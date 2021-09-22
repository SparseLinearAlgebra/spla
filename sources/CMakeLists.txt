set(SPLA_CPP_SOURCES
        sources/SplaData.cpp
        sources/SplaDescriptor.cpp
        sources/SplaExpression.cpp
        sources/SplaExpressionNode.cpp
        sources/SplaFunctionBinary.cpp
        sources/SplaFunctionUnary.cpp
        sources/SplaLibrary.cpp
        sources/SplaMatrix.cpp
        sources/SplaScalar.cpp
        sources/SplaType.cpp
        sources/SplaVector.cpp)

set(SPLA_DETAIL_SOURCES
        sources/detail/SplaError.hpp
        sources/detail/SplaLibraryPrivate.cpp
        sources/detail/SplaLibraryPrivate.hpp)

set(SPLA_EXPRESSION_SOURCES
        sources/expression/matrix/SplaMatrixDataWrite.cpp
        sources/expression/matrix/SplaMatrixDataWrite.hpp
        sources/expression/SplaExpressionContext.cpp
        sources/expression/SplaExpressionContext.hpp
        sources/expression/SplaExpressionManager.cpp
        sources/expression/SplaExpressionManager.hpp
        sources/expression/SplaNodeProcessor.hpp
        )