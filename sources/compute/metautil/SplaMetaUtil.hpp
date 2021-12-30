/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/JetBrains-Research/spla                                     */
/**********************************************************************************/
/* MIT License                                                                    */
/*                                                                                */
/* Copyright (c) 2021 JetBrains-Research                                          */
/*                                                                                */
/* Permission is hereby granted, free of charge, to any person obtaining a copy   */
/* of this software and associated documentation files (the "Software"), to deal  */
/* in the Software without restriction, including without limitation the rights   */
/* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell      */
/* copies of the Software, and to permit persons to whom the Software is          */
/* furnished to do so, subject to the following conditions:                       */
/*                                                                                */
/* The above copyright notice and this permission notice shall be included in all */
/* copies or substantial portions of the Software.                                */
/*                                                                                */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     */
/* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       */
/* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    */
/* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         */
/* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  */
/* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  */
/* SOFTWARE.                                                                      */
/**********************************************************************************/
#ifndef SPLA_SPLAMETAUTIL_HPP
#define SPLA_SPLAMETAUTIL_HPP

namespace spla::detail::meta {
    using boost::compute::uint_;
    using MetaKernel = boost::compute::detail::meta_kernel;

    struct DeclareMultiKey {
        const std::string Name{};
        const std::size_t KeySize{};
    };

    inline MetaKernel &operator<<(MetaKernel &k, const DeclareMultiKey &key) {
        for (std::size_t ki = 0; ki < key.KeySize; ++ki) {
            k << k.decl<uint_>(key.Name + std::to_string(ki)) << ";";
        }
        return k;
    }

    struct DeclareVal {
        const std::string Name{};
        const std::size_t vBytes{};
    };

    inline MetaKernel &operator<<(MetaKernel &k, const DeclareVal &v) {
        k << k.decl<unsigned char>(v.Name) << "[" << std::to_string(v.vBytes) << "];";
        return k;
    }

    class VMultiKey {
    public:
        [[nodiscard]] virtual std::string GetKeyByN(std::size_t n) const = 0;

        virtual ~VMultiKey() = default;
    };

    class KeyVar : public VMultiKey {
    public:
        explicit KeyVar(std::string name) : mName(std::move(name)) {}

        [[nodiscard]] std::string GetKeyByN(std::size_t n) const override {
            return mName + std::to_string(n);
        }

    private:
        const std::string mName{};
    };

    class KeyArrItem : public VMultiKey {
    public:
        explicit KeyArrItem(std::string arr, std::string idx) : mArr(std::move(arr)), mIdx(std::move(idx)) {}

        [[nodiscard]] std::string GetKeyByN(std::size_t n) const override {
            return mArr + std::to_string(n) + '[' + mIdx + ']';
        }

    private:
        const std::string mArr{};
        const std::string mIdx{};
    };

    class KeyVec : public VMultiKey {
    public:
        explicit KeyVec(
                const std::vector<std::reference_wrapper<const boost::compute::vector<uint_>>> &vectors,
                std::string idx,
                MetaKernel &k)
            : mIdx(std::move(idx)) {
            mBuffersNames.reserve(vectors.size());
            for (const boost::compute::vector<uint_> &v : vectors) {
                mBuffersNames.push_back(k.template get_buffer_identifier<uint_>(v.get_buffer()));
            }
        }

        explicit KeyVec(const std::vector<std::reference_wrapper<boost::compute::vector<uint_>>> &vectors,
                        std::string idx,
                        MetaKernel &k)
            : mIdx(std::move(idx)) {
            mBuffersNames.reserve(vectors.size());
            for (const boost::compute::vector<uint_> &v : vectors) {
                mBuffersNames.push_back(k.template get_buffer_identifier<uint_>(v.get_buffer()));
            }
        }

        explicit KeyVec(
                const std::vector<boost::compute::vector<uint_>::iterator> &iterators,
                std::string idx,
                MetaKernel &k)
            : mIdx(std::move(idx)) {
            mBuffersNames.reserve(iterators.size());
            for (const boost::compute::vector<uint_>::iterator &it : iterators) {
                mBuffersNames.push_back(k.template get_buffer_identifier<uint_>(it.get_buffer()));
            }
        }

        [[nodiscard]] std::string GetKeyByN(std::size_t n) const override {
            return mBuffersNames[n] + '[' + mIdx + ']';
        }

    private:
        std::vector<std::string> mBuffersNames{};
        const std::string mIdx{};
    };

    class VAlignedValue {
    public:
        [[nodiscard]] virtual std::string GetByteByN(const std::string &n) const = 0;

        [[nodiscard]] virtual std::string GetPointer() const = 0;

        virtual ~VAlignedValue() = default;
    };

    class ValVar : public VAlignedValue {
    public:
        explicit ValVar(std::string name) : mName(std::move(name)) {}

        [[nodiscard]] std::string GetByteByN(const std::string &n) const override {
            return mName + '[' + n + ']';
        }

        [[nodiscard]] std::string GetPointer() const override {
            return mName;
        }

    private:
        std::string mName{};
    };

    class ValArrItem : public VAlignedValue {
    public:
        explicit ValArrItem(std::string arr, std::string idx, std::size_t vBytes)
            : mArr(std::move(arr)), mIdx(std::move(idx)), mVBytes(vBytes) {}

        explicit ValArrItem(const boost::compute::vector<unsigned char>::iterator &it, std::string idx, std::size_t vBytes, MetaKernel &k)
            : mArr(k.get_buffer_identifier<unsigned char>(it.get_buffer())),
              mIdx(std::move(idx)), mVBytes(vBytes) {}


        explicit ValArrItem(const boost::compute::vector<unsigned char> &v, std::string idx, std::size_t vBytes, MetaKernel &k)
            : mArr(k.get_buffer_identifier<unsigned char>(v.get_buffer())),
              mIdx(std::move(idx)), mVBytes(vBytes) {}

        [[nodiscard]] std::string GetByteByN(const std::string &n) const override {
            std::stringstream ss;
            ss << mArr << "[(" << mIdx << ") * " << mVBytes << " + (" << n << ")]";
            return ss.str();
        }

        [[nodiscard]] std::string GetPointer() const override {
            std::stringstream ss;
            ss << '&' << mArr << "[(" << mIdx << ") * " << mVBytes << ']';
            return ss.str();
        }

    private:
        std::string mArr{};
        std::string mIdx{};
        std::size_t mVBytes{};
    };

    struct AssignKey {
        const VMultiKey &Left, &Right;
        const std::size_t KeySize{};
    };

    inline MetaKernel &operator<<(MetaKernel &k, const AssignKey &key) {
        for (std::size_t ki = 0; ki < key.KeySize; ++ki) {
            std::string ind = std::to_string(ki);
            k << key.Left.GetKeyByN(ki) << " = " << key.Right.GetKeyByN(ki) << ";";
        }
        return k;
    }

    struct CompareKey {
        const VMultiKey &Left, &Right;
        const std::size_t KeySize{};
    };

    inline MetaKernel &operator<<(MetaKernel &k, const CompareKey &comp) {
        for (std::size_t ki = 0; ki < comp.KeySize; ++ki) {
            std::string ind = std::to_string(ki);
            k << '(' << comp.Left.GetKeyByN(ki) << " == " << comp.Right.GetKeyByN(ki) << ")";
            if (ki + 1 < comp.KeySize) {
                k << " && ";
            }
        }
        return k;
    }

    struct AssignVal {
        const VAlignedValue &Left, &Right;
        const std::size_t vBytes{};
    };

    inline MetaKernel &operator<<(MetaKernel &k, const AssignVal &v) {
        k << "for (uint byte_i = 0; byte_i < " << std::to_string(v.vBytes) << "; byte_i++) {\n"
          << v.Left.GetByteByN("byte_i") << " = " << v.Right.GetByteByN("byte_i") << ";\n"
          << "}\n";
        return k;
    }

    class ReduceApplication {
    public:
        explicit ReduceApplication(std::string opName,
                                   const VAlignedValue &a,
                                   const VAlignedValue &b,
                                   const VAlignedValue &c)
            : mOpName(std::move(opName)), mA(a), mB(b), mC(c) {}

        virtual void Print(MetaKernel &) const = 0;

        virtual ~ReduceApplication() = default;

    protected:
        const std::string mOpName{};
        const VAlignedValue &mA, &mB, &mC;
    };

    class ReduceApplicationNonRestrict : public ReduceApplication {
    private:
        static inline const char *REDUCE_RESULT_VAR = "reduce_result";

    public:
        explicit ReduceApplicationNonRestrict(std::string opName,
                                              const VAlignedValue &a,
                                              const VAlignedValue &b,
                                              const VAlignedValue &c,
                                              std::size_t vBytes)
            : ReduceApplication(std::move(opName), a, b, c), mVBytes(vBytes) {}

        void Print(MetaKernel &k) const override {
            k << "{\n"
              << DeclareVal{REDUCE_RESULT_VAR, mVBytes} << ";\n"
              << mOpName << '(' << mA.GetPointer() << ", " << mB.GetPointer() << ", " << REDUCE_RESULT_VAR << ')' << ";\n"
              << AssignVal{mC, ValVar(REDUCE_RESULT_VAR), mVBytes} << ";\n"
              << "}\n";
        }

    private:
        const std::size_t mVBytes{};
    };

    class ReduceApplicationRestrict : public ReduceApplication {
    public:
        using ReduceApplication::ReduceApplication;

        void Print(MetaKernel &k) const override {
            k << mOpName << '(' << mA.GetPointer() << ", " << mB.GetPointer() << ", " << mC.GetPointer() << ')' << ";\n";
        }
    };

    inline MetaKernel &operator<<(MetaKernel &k, const std::shared_ptr<ReduceApplication> &reduce) {
        reduce->Print(k);
        return k;
    }

    enum class Visibility {
        Unspecified,
        Local,
        Global
    };

    inline std::string PrintVisibility(Visibility v) {
        std::string s;
        switch (v) {
            case (Visibility::Unspecified):
                s = "";
                break;
            case (Visibility::Local):
                s = "__local";
                break;
            case (Visibility::Global):
                s = "__global";
                break;
            default:
                break;
        }
        return s;
    };

    inline std::string MakeFunction(const std::string &name, const std::string &body, Visibility accessA, Visibility accessB, Visibility accessC) {
        std::stringstream fun;
        fun << "void " << name << " ("
            << PrintVisibility(accessA) << " const void* vp_a, "
            << PrintVisibility(accessB) << " const void* vp_b, "
            << PrintVisibility(accessC) << " void* vp_c) {\n"
            << "#define _ACCESS_A " << PrintVisibility(accessA) << "\n"
            << "#define _ACCESS_B " << PrintVisibility(accessB) << "\n"
            << "#define _ACCESS_C " << PrintVisibility(accessC) << "\n"
            << body << "\n"
            << "#undef _ACCESS_A\n"
            << "#undef _ACCESS_B\n"
            << "#undef _ACCESS_C\n"
            << "}";
        return fun.str();
    }

    class FunctionApplication {
    public:
        explicit FunctionApplication(MetaKernel &k,
                                     std::string reduceOpName,
                                     const std::string &body,
                                     std::size_t vBytes,
                                     Visibility aVisibility = Visibility::Unspecified,
                                     Visibility bVisibility = Visibility::Unspecified,
                                     Visibility cVisibility = Visibility::Unspecified)
            : FunctionApplication(k, reduceOpName, body, std::optional(vBytes), aVisibility, bVisibility, cVisibility) {}

        explicit FunctionApplication(MetaKernel &k,
                                     std::string reduceOpName,
                                     const std::string &body,
                                     Visibility aVisibility = Visibility::Unspecified,
                                     Visibility bVisibility = Visibility::Unspecified,
                                     Visibility cVisibility = Visibility::Unspecified)
            : FunctionApplication(k, reduceOpName, body, std::nullopt, aVisibility, bVisibility, cVisibility) {}

        std::shared_ptr<ReduceApplication> Apply(const VAlignedValue &left,
                                                 const VAlignedValue &right,
                                                 const VAlignedValue &result) {
            std::string lPtr = left.GetPointer();
            std::string rPtr = right.GetPointer();
            std::string resPtr = result.GetPointer();
            if (lPtr != resPtr && rPtr != resPtr) {
                return std::make_shared<ReduceApplicationRestrict>(mReduceOpName, left, right, result);
            }
            assert(mVBytes.has_value() && "To invoke function (non-restrict) value byte size is required");
            return std::make_shared<ReduceApplicationNonRestrict>(mReduceOpName, left, right, result, mVBytes.value());
        }

    private:
        explicit FunctionApplication(MetaKernel &k,
                                     std::string reduceOpName,
                                     const std::string &body,
                                     std::optional<std::size_t> vBytes,
                                     Visibility aVisibility,
                                     Visibility bVisibility,
                                     Visibility cVisibility)
            : mReduceOpName(std::move(reduceOpName)),
              mVBytes(vBytes) {

            std::stringstream splaReduceOp;

            std::string aAccess = PrintVisibility(aVisibility);
            std::string bAccess = PrintVisibility(bVisibility);
            std::string cAccess = PrintVisibility(cVisibility);

            splaReduceOp << MakeFunction(mReduceOpName,
                                         body,
                                         aVisibility,
                                         bVisibility,
                                         cVisibility);

            k.add_function(mReduceOpName, splaReduceOp.str());
        }

        const std::string mReduceOpName{};
        const std::optional<std::size_t> mVBytes{};
    };

}// namespace spla::detail::meta

#endif//SPLA_SPLAMETAUTIL_HPP
