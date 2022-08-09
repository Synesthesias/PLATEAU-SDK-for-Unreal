#pragma once

#include <stdexcept>
#include <string>

class MeshCode {
public:
    explicit MeshCode(std::string code)
        : code_(std::move(code)) {
    }


    int getLevel() const {
        if (code_.size() == 6) {
            return 2;
        }
        if (code_.size() == 8) {
            return 3;
        }
        // 2次メッシュ、3次メッシュ以外はサポート対象外
        throw std::runtime_error("Invalid string for regional mesh code");
    }

    std::string get() const {
        return code_;
    }

    std::string getAsLevel2() const {
        return code_.substr(0, 6);
    }

    bool operator==(const MeshCode& other) const {
        return get() == other.get();
    }

private:
    std::string code_;
};
