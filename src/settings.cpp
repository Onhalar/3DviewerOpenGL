#include <config.hpp>

#include <FormatConsole.hpp>
#include <paths.hpp>

#include <json.hpp>

#include <variant>
#include <unordered_map>
#include <tuple>

#include <updateRunningConfig.hpp>

using Color = tuple<float, float, float, float>;
using AnyType = variant<float*, int*, bool*, string*, Nanoseconds*, Color*>;
using Json = nlohmann::json;
using JsonSetter = void(*)(AnyType, const char*, const Json&);

void setString(AnyType variable, const char* jsonNameIndex, const Json&);
void setInt(AnyType variable, const char* jsonNameIndex, const Json&);
void setTuple(AnyType variable, const char* jsonNameIndex, const Json&);
void setBool(AnyType variable, const char* jsonNameIndex, const Json&);
void setFloat(AnyType variable, const char* jsonNameIndex, const Json&);
void setNanoseconds(AnyType variable, const char* jsonNameIndex, const Json&);

struct SettingsEntry {
    AnyType variable;
    JsonSetter setter;

    SettingsEntry() = default;

    SettingsEntry(AnyType var, JsonSetter set) {
        variable = var; setter = set;
    }
};

std::unordered_map<std::string, SettingsEntry> settings;

void loadSettings(filesystem::path path) {
    bool showMessages;
    if (debugMode) {
        cout << formatProcess("Loading") << " settings '" << formatPath(getFileName(path.string())) << "' ... ";
        showMessages = true;
    }

    // register setting variables
    settings["debugMode"] = SettingsEntry(&debugMode, setBool);
    settings["maxFrameRate"] = SettingsEntry(&maxFrameRate, setInt);
    settings["defaultWindowWidth"] = SettingsEntry(&defaultWindowWidth, setInt);
    settings["defaultWindowHeight"] = SettingsEntry(&defaultWindowHeight, setInt);
    settings["minWindowWidth"] = SettingsEntry(&minWindowWidth, setInt);
    settings["minWindowHeight"] = SettingsEntry(&minWindowHeight, setInt);
    settings["defaultBackgroundColor"] = SettingsEntry(&defaultBackgroundColor, setTuple);
    settings["prettyOutput"] = SettingsEntry(&prettyOutput, setBool);
    settings["VSync"] = SettingsEntry(&VSync, setInt);
    settings["StaticFrameDelayFraction"] = SettingsEntry(&staticDelayFraction, setFloat);
    settings["spinDelayNS"] = SettingsEntry(&spinDelay, setNanoseconds);

    if (!filesystem::exists(path)) {
        cout << formatError("FAILED") << "\n";
        cerr << "unable to open '" << formatPath(path.string()) << "'\n" << endl;
        return;
    }

    ifstream file(path);
    Json data;
    file >> data;
    file.close();

    for (auto& entry : data.items()) {
        const char* key = entry.key().c_str();
        if (settings.find(key) != settings.end()) {
            settings[key].setter(settings[key].variable, key, data);
        }
    }

    updateRunningConfig::updateAllFromSet();

    if (showMessages) { cout << formatSuccess("Done") << endl; }
}




void setString(AnyType variable, const char* jsonNameIndex, const Json& data) {
    *std::get<std::string*>(variable) = data[jsonNameIndex].get<std::string>();
}
void setInt(AnyType variable, const char* jsonNameIndex, const Json& data) {
    *std::get<int*>(variable) = data[jsonNameIndex].get<int>();
}
void setTuple(AnyType variable, const char* jsonNameIndex, const Json& data) {
    const auto& bgArray = data[jsonNameIndex];
    *std::get<Color*>(variable) = (Color){bgArray[0].get<float>(), bgArray[1].get<float>(), bgArray[2].get<float>(), bgArray[3].get<float>()};
}
void setBool(AnyType variable, const char* jsonNameIndex, const Json& data) {
    *std::get<bool*>(variable) = data[jsonNameIndex].get<bool>();
}
void setFloat(AnyType variable, const char* jsonNameIndex, const Json& data) {
    *std::get<float*>(variable) = data[jsonNameIndex].get<float>();
}
void setNanoseconds(AnyType variable, const char* jsonNameIndex, const Json& data) {
    *std::get<Nanoseconds*>(variable) = Nanoseconds(data[jsonNameIndex].get<int>());
}