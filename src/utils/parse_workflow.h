#ifndef PARSE_WORKFLOW_H
#define PARSE_WORKFLOW_H

#include <string>
#include <stdexcept>
#include <fstream>
#include <vector>
#include <filesystem>

#include "rapidjson/document.h"
#include "hdag_utils.h"

namespace hive {
namespace utils {

bool isJsonFile(std::string filename)
{
    std::string extension = filename.substr(filename.find_last_of(".") + 1);
    if (extension == "json") return true;
    else return false;
}

class Parser 
{
private:
    HDAG hive_dag = {};
public:
    explicit Parser(const std::string &path) {
        bool isJson = isJsonFile(path);
        if(!isJson) throw std::invalid_argument("The file is not a json file");

        std::ifstream f(path);
        if (!f.is_open()) {
            throw std::runtime_error("Failed to open the file.");
        }

        // Read the file content into a string
        std::string jsonStr((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

        rapidjson::Document document;
        document.Parse(jsonStr.c_str());

        if (document.HasParseError() || !document.IsObject()) {
            throw std::runtime_error("Failed to parse the JSON file.");
        }

        // Get the Workflow object
        const rapidjson::Value& app = document["application"];
        const rapidjson::Value& workflowid = document["Workflow_id"];
        this->hive_dag.app_ = app.GetString();
        this->hive_dag.name_ = workflowid.GetString();

        const rapidjson::Value& workflow = document["Workflow"];
        if (!workflow.IsObject()) {
            throw std::runtime_error("[WORKFLOW] Invalid JSON structure.");
        }

        // Iterate over the stages
        int stageId = 0;
        for(auto stageItr = workflow.MemberBegin(); stageItr != workflow.MemberEnd(); ++stageItr, ++stageId) {
            std::string stageName = stageItr->name.GetString();
            const rapidjson::Value& stage = stageItr->value;

            if (!stage.IsObject()) {
                throw std::runtime_error("[STAGE] Invalid JSON structure.");
            }

            for(auto stateItr = stage.MemberBegin(); stateItr != stage.MemberEnd(); ++stateItr) {
                std::string stateName = stateItr->name.GetString();
                const rapidjson::Value& state = stateItr->value;

                if (!state.IsObject()) {
                    throw std::runtime_error("[STATE] Invalid JSON structure.");
                }

                std::string id = state["id"].GetString();
                bool start = state["start"].GetBool();
                std::string function_name = state["function"].GetString();

                const rapidjson::Value& input = state["input"];
                if (!input.IsArray()) {
                    throw std::runtime_error("[INPUT] Invalid JSON structure.");
                }
                std::vector<std::string> inputList;
                for (rapidjson::SizeType i = 0; i < input.Size(); i++) {
                    inputList.push_back(input[i].GetString());
                }

                const rapidjson::Value& output = state["output"];
                if (!output.IsArray()) {
                    throw std::runtime_error("[OUTPUT] Invalid JSON structure.");
                }
                std::vector<std::string> outputList;
                for (rapidjson::SizeType i = 0; i < output.Size(); i++) {
                    outputList.push_back(output[i].GetString());
                }

                const rapidjson::Value& next = state["next"];
                if (!next.IsArray()) {
                    throw std::runtime_error("[NEXT] Invalid JSON structure.");
                }
                std::vector<std::string> nextList;
                for (rapidjson::SizeType i = 0; i < next.Size(); i++) {
                    nextList.push_back(next[i].GetString());
                }

                func_node *node = new func_node(stateName, id, stageName, stageId, start, inputList, outputList, nextList);
                this->hive_dag.func_list.push_back(node);
            }
        }
        this->hive_dag.construct();
    }

    Parser() = default;

    void parse(const std::string &workflow_content) {
        rapidjson::Document document;
        document.Parse(workflow_content.c_str());

        if (document.HasParseError() || !document.IsObject()) {
            throw std::runtime_error("Failed to parse the JSON file.");
        }

        // Get the Workflow object
        const rapidjson::Value& app = document["application"];
        const rapidjson::Value& workflowid = document["Workflow_id"];
        this->hive_dag.app_ = app.GetString();
        this->hive_dag.name_ = workflowid.GetString();

        const rapidjson::Value& workflow = document["Workflow"];
        if (!workflow.IsObject()) {
            throw std::runtime_error("[WORKFLOW] Invalid JSON structure.");
        }

        // Iterate over the stages
        int stageId = 0;
        for(auto stageItr = workflow.MemberBegin(); stageItr != workflow.MemberEnd(); ++stageItr, ++stageId) {
            std::string stageName = stageItr->name.GetString();
            const rapidjson::Value& stage = stageItr->value;

            if (!stage.IsObject()) {
                throw std::runtime_error("[STAGE] Invalid JSON structure.");
            }

            for(auto stateItr = stage.MemberBegin(); stateItr != stage.MemberEnd(); ++stateItr) {
                std::string stateName = stateItr->name.GetString();
                const rapidjson::Value& state = stateItr->value;

                if (!state.IsObject()) {
                    throw std::runtime_error("[STATE] Invalid JSON structure.");
                }

                std::string id = state["id"].GetString();
                bool start = state["start"].GetBool();
                std::string function_name = state["function"].GetString();

                const rapidjson::Value& input = state["input"];
                if (!input.IsArray()) {
                    throw std::runtime_error("[INPUT] Invalid JSON structure.");
                }
                std::vector<std::string> inputList;
                for (rapidjson::SizeType i = 0; i < input.Size(); i++) {
                    inputList.push_back(input[i].GetString());
                }

                const rapidjson::Value& output = state["output"];
                if (!output.IsArray()) {
                    throw std::runtime_error("[OUTPUT] Invalid JSON structure.");
                }
                std::vector<std::string> outputList;
                for (rapidjson::SizeType i = 0; i < output.Size(); i++) {
                    outputList.push_back(output[i].GetString());
                }

                const rapidjson::Value& next = state["next"];
                if (!next.IsArray()) {
                    throw std::runtime_error("[NEXT] Invalid JSON structure.");
                }
                std::vector<std::string> nextList;
                for (rapidjson::SizeType i = 0; i < next.Size(); i++) {
                    nextList.push_back(next[i].GetString());
                }

                func_node *node = new func_node(stateName, id, stageName, stageId, start, inputList, outputList, nextList);
                this->hive_dag.func_list.push_back(node);
            }
        }
        this->hive_dag.construct();
    }

    const HDAG& get_hive_dag() {
        return this->hive_dag;
    }
};

}
}

#endif //PARSE_WORKFLOW_H