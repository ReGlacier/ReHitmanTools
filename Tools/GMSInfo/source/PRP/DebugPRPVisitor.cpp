#include <PRP/DebugPRPVisitor.h>

#include <spdlog/spdlog.h>

#include <sstream>
#include <iomanip>

namespace ReGlacier
{
    namespace Helpers {
        static std::string FormatByteArrayAsString(const uint8_t* buffer, size_t size, int align = 8)
        {
            std::stringstream ss;

            for (size_t i = 0; i < size; i++)
            {
                if (i == 0) { ss << '\t'; }
                else if (i % align == 0) { ss << "\n\t"; }
                else {
                    ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(buffer[i]) << ' ';
                }
            }

            return ss.str();
        }

        static std::string JoinStringArray(const std::vector<std::string>& arr, char ch = ',') {
            std::string result;

            const size_t totalElements = arr.size();
            for (size_t i = 0; i < totalElements; i++) {
                if (i + 1 < totalElements) {
                    result.append(fmt::format("{},", arr[i]));
                }
                else {
                    result.append(arr[i]);
                }
            }

            return result;
        }
    }

    void DebugPRPVisitor::Visit_BeginArray(uint32_t capacity) { spdlog::info("DebugPRPVisitor| Array({})", capacity); }
    void DebugPRPVisitor::Visit_EndArray() { spdlog::info("DebugPRPVisitor| End Array"); }
    void DebugPRPVisitor::Visit_BeginObject() { spdlog::info("DebugPRPVisitor| Begin Object"); }
    void DebugPRPVisitor::Visit_EndObject() { spdlog::info("DebugPRPVisitor| End Object"); }
    void DebugPRPVisitor::Visit_Container(uint32_t v) { spdlog::info("DebugPRPVisitor| Container ({})", v); }
    void DebugPRPVisitor::Visit_Bitfield(uint32_t v) { spdlog::info("DebugPRPVisitor| Bitfield {}", v); }
    void DebugPRPVisitor::Visit_Bool(bool v) { spdlog::info("DebugPRPVisitor| Bool {}", (v ? "True": "False")); }
    void DebugPRPVisitor::Visit_Char(char v) { spdlog::info("DebugPRPVisitor| Char '{}'", v); }
    void DebugPRPVisitor::Visit_I8(uint8_t v) { spdlog::info("DebugPRPVisitor| Int8 {}", v); }
    void DebugPRPVisitor::Visit_I16(uint16_t v) { spdlog::info("DebugPRPVisitor| Int16 {}", v); }
    void DebugPRPVisitor::Visit_I32(uint32_t v) { spdlog::info("DebugPRPVisitor| Int32 {}", v); }
    void DebugPRPVisitor::Visit_F32(float v) { spdlog::info("DebugPRPVisitor| Float32 {:f}", v); }
    void DebugPRPVisitor::Visit_F64(double v) { spdlog::info("DebugPRPVisitor| Float64 {}", v); }
    void DebugPRPVisitor::Visit_String(const std::string& v) { spdlog::info("DebugPRPVisitor| String \"{}\"", v); }
    void DebugPRPVisitor::Visit_RawBuffer(const std::shared_ptr<uint8_t[]>& buffer, size_t& bufferSize) {
        spdlog::info(
                "DebugPRPVisitor| Raw Data ({} entities):\n{}",
                bufferSize,
                Helpers::FormatByteArrayAsString(buffer.get(), bufferSize)
        );
    }
    void DebugPRPVisitor::Visit_StringArray(const std::vector<std::string>& stringArray) {
        spdlog::info(
                "DebugPRPVisitor| String Array ({} entities): {}",
                stringArray.size(),
                Helpers::JoinStringArray(stringArray)
        );
    }
    void DebugPRPVisitor::Visit_StringOrArray(const std::string& s) { spdlog::info("DebugPRPVisitor| StringOrArray (str): {}", s); }
    void DebugPRPVisitor::Visit_StringOrArray(uint32_t v) { spdlog::info("DebugPRPVisitor| StringOrArray (i32): {}", v); }
    void DebugPRPVisitor::Visit_SkipMark() { spdlog::info("DebugPRPVisitor| Skip mark"); }
    void DebugPRPVisitor::Visit_EndOfStream() { spdlog::info("DebugPRPVisitor| End of stream"); }
    void DebugPRPVisitor::Visit_UnknownTag(uint8_t v) { spdlog::warn("DebugPRPVisitor| Unknown tag {:2X}", v); }
}