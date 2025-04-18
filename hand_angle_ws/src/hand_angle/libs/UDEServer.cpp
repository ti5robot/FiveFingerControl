#include "hand_angle/UDEServer.h"


const string GloveDataHeaders[30] = 
{
    "LeftThumb1", 
    "LeftThumb2", 
    "LeftThumb3", 
    "LeftIndex1", 
    "LeftIndex2", 
    "LeftIndex3", 
    "LeftMiddle1",
    "LeftMiddle2",
    "LeftMiddle3",
    "LeftRing1",
    "LeftRing2",
    "LeftRing3",
    "LeftPinky1", 
    "LeftPinky2", 
    "LeftPinky3", 
    "RightThumb1", 
    "RightThumb2", 
    "RightThumb3", 
    "RightIndex1", 
    "RightIndex2", 
    "RightIndex3",
    "RightMiddle1",
    "RightMiddle2",
    "RightMiddle3",
    "RightRing1",
    "RightRing2",
    "RightRing3",
    "RightPinky1", 
    "RightPinky2", 
    "RightPinky3", 
};

const string ControllerHeaders[12] = 
{
    "Left Joy X",
    "Left Joy Y",
    "Left A Button",
    "Left B Button",
    "Left Joy Button",
    "Left Menu Button",
    "Right Joy X",
    "Right Joy Y",
    "Right A Button",
    "Right B Button",
    "Right Joy Button",
    "Right Menu Button"
};

vector<GloveData> UDEGloveSDK::gloveDataList;
vector<GloveControllerData> UDEGloveSDK::gloveControllerDataList;

int UDEGloveSDK::Initialize()
{
    sock_fd = socket(AF_INET , SOCK_DGRAM , 0); 
    if(sock_fd < 0){
        perror("failed to open socket");
        return -1;
    }

    /** 绑定IP和端口号 */ 
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // 本地任意IP
    server_addr.sin_port = htons(UDEGloveSDK::Port);   // 指定端口号
    int ret = bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(ret < 0)
    {
        perror("failed to bind");
        close(sock_fd);
        return -1;
    }

    // client_addr.sin_family = AF_INET;
    // client_addr.sin_addr.s_addr = inet_addr(Send_IP);
    // client_addr.sin_port = htons(8920);
    // int len = sizeof(client_addr);
    
    /** 设置接收超时（可选） */
    int mill_sec = 2000;  // 毫秒
    struct timeval time_out;
    time_out.tv_sec = mill_sec / 1000;
    time_out.tv_usec = (mill_sec- time_out.tv_sec * 1000) * 1000;
    ret = setsockopt(sock_fd, SOL_SOCKET,SO_RCVTIMEO,&time_out,sizeof (timeval));
    if(ret < 0)
    {
        perror("udp setTimeOut error!");
    }
    
    for(int i = 0; i < 30; ++i)
    {
        GloveVecRes.push_back(Vector3Float(0,0,0));
    }

    UDEGloveSDK::CurStatus = ServerStatus::READY;

    return 0;
}

void UDEGloveSDK::recv_func(int sock_fd_)
{
    char buffer[1024 * 1024];
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    while(true)
    {
        memset(buffer, 0, sizeof(buffer));
        ssize_t len = recvfrom(sock_fd_, buffer, sizeof(buffer), 0, reinterpret_cast<struct sockaddr *>(&addr), &addr_len);
        if(len > 0)
        {
            std::string str(reinterpret_cast<const char*>(buffer));
            Json::Value value;
            const auto rawJsonLength = static_cast<int>(str.length());
            JSONCPP_STRING err;
            {
                Json::CharReaderBuilder builder;
                const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
                if(!reader->parse(str.c_str(), str.c_str() + rawJsonLength, &value, &err))
                {
                    cout << "Error" << endl;
                    continue;
                }
            }

            Json::Value::Members member_names = value.getMemberNames();
            if((int)member_names.size() == 0)
            {
                continue;
            }
            UDEGloveSDK::gloveDataList.clear();
            UDEGloveSDK::gloveControllerDataList.clear();

            for(int i = 0; i < (int)member_names.size(); ++i)
            {
                GloveData gloveData;
                GloveControllerData controllerData;
                gloveData.roleName = member_names[i];
                controllerData.roleName = member_names[i];

                Json::Value device = value[member_names[i]];
                Json::Value parameter = device["Parameter"];
                for(unsigned int index = 0; index < (int)parameter.size(); ++index)
                {
                    string Name = parameter[index]["Name"].asString();
                    if(Name[1] == '_' && (Name[0] == 'l' || Name[0] == 'r'))  //controller value
                    {
                        if (Name[(int)Name.size() - 1] == 'X' || Name[(int)Name.size() - 1] == 'Y') //Joystick Value
                        {
                            Parameter item;
                            item.name = Name;
                            item.value = (float)stod(parameter[index]["Value"].asString());
                            if (Name[0] == 'l')
                            {
                                controllerData.controllerDatas.JoyValue_L.push_back(item);
                            }
                            else
                            {
                                controllerData.controllerDatas.JoyValue_R.push_back(item);
                            }
                        }
                        else
                        {
                            Parameter item;
                            item.name = Name;
                            item.value = parameter[index]["Value"].asString() == "true" ? 1.f : 0.f;
                            if (Name[0] == 'l')
                            {
                                controllerData.controllerDatas.ButtonValue_L.push_back(item);
                            }
                            else
                            {
                                controllerData.controllerDatas.ButtonValue_R.push_back(item);
                            }
                        }
                    }
                    else
                    {
                        Parameter item;
                        item.name = Name;
                        item.value = stod(parameter[index]["Value"].asString());
                        if (item.name[0] == 'l' || item.name[0] == 'L')
                        {
                            gloveData.handDatas.fingerJoints.fingerJoint_L.push_back(item);
                        }
                        else
                        {
                            gloveData.handDatas.fingerJoints.fingerJoint_R.push_back(item);
                        }
                    }
                }
                UDEGloveSDK::gloveDataList.push_back(gloveData);
                UDEGloveSDK::gloveControllerDataList.push_back(controllerData);
            }

        }else{
            perror("recv error");
        }
    }
}

// void UDEGloveSDK::send_func(int sock_fd_)
// {
//     unsigned char send_buffer[1024] = "Linux Sending";

//     auto send_num = sendto(sock_fd_, send_buffer, strlen(send_buffer), 0, reinterpret_cast<struct sockaddr *>(&client_addr), sizeof(client_addr))
//     if(send_num < 0)
//     {
//         perror("sendto error: ");
//         exit(1);
//     }
// }

vector<string> UDEGloveSDK::GetRoleNameList()
{
    UDEGloveSDK::NameList.clear();
    for(auto &item : gloveDataList)
    {
        UDEGloveSDK::NameList.push_back(item.roleName);
    }
    return UDEGloveSDK::NameList;
}

void UDEGloveSDK::SetPortNum(int port)
{
    UDEGloveSDK::Port = port;
    Initialize();
}

void UDEGloveSDK::StartListening()
{
    UDEGloveSDK::recv_t = thread(UDEGloveSDK::recv_func, UDEGloveSDK::sock_fd);
    recv_t.detach();
    UDEGloveSDK::CurStatus = ServerStatus::IN_LISTENING;
}

void UDEGloveSDK::EndListening()
{
    UDEGloveSDK::recv_t.join();
    UDEGloveSDK::CurStatus = ServerStatus::END;
}

ServerStatus UDEGloveSDK::GetStatus()
{
    return UDEGloveSDK::CurStatus;
}

map<string, vector<float>> UDEGloveSDK::GetFingerData(string RoleName)
{
    GloveRes.clear();
    for(auto &gloves: gloveDataList)
    {
        if(gloves.roleName == RoleName)
        {
            auto joints = gloves.handDatas.fingerJoints.fingerJoint_L;

            GloveRes.insert({"LeftThumb1", {joints[HandJointIndex::Thumb1X].value, joints[HandJointIndex::Thumb1Y].value, joints[HandJointIndex::Thumb1Z].value}});
            GloveRes.insert({"LeftThumb2", {joints[HandJointIndex::Thumb2X].value, 0, 0}});
            GloveRes.insert({"LeftThumb3", {joints[HandJointIndex::Thumb3X].value, 0, 0}});
            GloveRes.insert({"LeftIndex1", {joints[HandJointIndex::Index1X].value, joints[HandJointIndex::Index1Y].value, joints[HandJointIndex::Index1Z].value}});
            GloveRes.insert({"LeftIndex2", {joints[HandJointIndex::Index2X].value, 0, 0}});
            GloveRes.insert({"LeftIndex3", {joints[HandJointIndex::Index3X].value, 0, 0}});
            GloveRes.insert({"LeftMiddle1", {joints[HandJointIndex::Middle1X].value, joints[HandJointIndex::Middle1Y].value, 0}});
            GloveRes.insert({"LeftMiddle2", {joints[HandJointIndex::Middle2X].value, 0, 0}});
            GloveRes.insert({"LeftMiddle3", {joints[HandJointIndex::Middle3X].value, 0, 0}});
            GloveRes.insert({"LeftRing1", {joints[HandJointIndex::Ring1X].value, joints[HandJointIndex::Ring1Y].value, 0}});
            GloveRes.insert({"LeftRing2", {joints[HandJointIndex::Ring2X].value, 0, 0}});
            GloveRes.insert({"LeftRing3", {joints[HandJointIndex::Ring3X].value, 0, 0}});
            GloveRes.insert({"LeftPinky1", {joints[HandJointIndex::Pinky1X].value, joints[HandJointIndex::Pinky1Y].value, joints[HandJointIndex::Pinky1Z].value}});
            GloveRes.insert({"LeftPinky2", {joints[HandJointIndex::Pinky2X].value, 0, 0}});
            GloveRes.insert({"LeftPinky3", {joints[HandJointIndex::Pinky3X].value, 0, 0}});

            joints = gloves.handDatas.fingerJoints.fingerJoint_R;

            GloveRes.insert({"RightThumb1", {joints[HandJointIndex::Thumb1X].value, joints[HandJointIndex::Thumb1Y].value, joints[HandJointIndex::Thumb1Z].value}});
            GloveRes.insert({"RightThumb2", {joints[HandJointIndex::Thumb2X].value, 0, 0}});
            GloveRes.insert({"RightThumb3", {joints[HandJointIndex::Thumb3X].value, 0, 0}});
            GloveRes.insert({"RightIndex1", {joints[HandJointIndex::Index1X].value, joints[HandJointIndex::Index1Y].value, joints[HandJointIndex::Index1Z].value}});
            GloveRes.insert({"RightIndex2", {joints[HandJointIndex::Index2X].value, 0, 0}});
            GloveRes.insert({"RightIndex3", {joints[HandJointIndex::Index3X].value, 0, 0}});
            GloveRes.insert({"RightMiddle1", {joints[HandJointIndex::Middle1X].value, joints[HandJointIndex::Middle1Y].value, 0}});
            GloveRes.insert({"RightMiddle2", {joints[HandJointIndex::Middle2X].value, 0, 0}});
            GloveRes.insert({"RightMiddle3", {joints[HandJointIndex::Middle3X].value, 0, 0}});
            GloveRes.insert({"RightRing1", {joints[HandJointIndex::Ring1X].value, joints[HandJointIndex::Ring1Y].value, 0}});
            GloveRes.insert({"RightRing2", {joints[HandJointIndex::Ring2X].value, 0, 0}});
            GloveRes.insert({"RightRing3", {joints[HandJointIndex::Ring3X].value, 0, 0}});
            GloveRes.insert({"RightPinky1", {joints[HandJointIndex::Pinky1X].value, joints[HandJointIndex::Pinky1Y].value, joints[HandJointIndex::Pinky1Z].value}});
            GloveRes.insert({"RightPinky2", {joints[HandJointIndex::Pinky2X].value, 0, 0}});
            GloveRes.insert({"RightPinky3", {joints[HandJointIndex::Pinky3X].value, 0, 0}});

            return GloveRes;
        }
    }
    return GloveRes;
}

map<string, float> UDEGloveSDK::GetControllerData(string RoleName)
{
    ControllerMapRes.clear();
    for(auto &gloves: gloveControllerDataList)
    {
        if(gloves.roleName == RoleName)
        {
            ControllerMapRes.insert({"Left Joy X", gloves.controllerDatas.JoyValue_L[0].value});
            ControllerMapRes.insert({"Left Joy Y", gloves.controllerDatas.JoyValue_L[1].value});
            ControllerMapRes.insert({"Left A Button", gloves.controllerDatas.ButtonValue_L[0].value});
            ControllerMapRes.insert({"Left B Button", gloves.controllerDatas.ButtonValue_L[1].value});
            ControllerMapRes.insert({"Left Joy Button", gloves.controllerDatas.ButtonValue_L[2].value});
            ControllerMapRes.insert({"Left Menu Button", gloves.controllerDatas.ButtonValue_L[3].value});

            ControllerMapRes.insert({"Right Joy X", gloves.controllerDatas.JoyValue_R[0].value});
            ControllerMapRes.insert({"Right Joy Y", gloves.controllerDatas.JoyValue_R[1].value});
            ControllerMapRes.insert({"Right A Button", gloves.controllerDatas.ButtonValue_R[0].value});
            ControllerMapRes.insert({"Right B Button", gloves.controllerDatas.ButtonValue_R[1].value});
            ControllerMapRes.insert({"Right Joy Button", gloves.controllerDatas.ButtonValue_R[2].value});
            ControllerMapRes.insert({"Right Menu Button", gloves.controllerDatas.ButtonValue_R[3].value});
            break;
        }
    }
    return ControllerMapRes;
}

vector<Vector3Float> UDEGloveSDK::GetVecFingerData(string RoleName)
{
    if(gloveDataList.size() == 0) return GloveVecRes;
    for(auto &gloves: gloveDataList)
    {
        if(gloves.roleName == RoleName)
        {
            auto joints = gloves.handDatas.fingerJoints.fingerJoint_L;

            GloveVecRes[0] = Vector3Float(joints[HandJointIndex::Thumb1X].value, joints[HandJointIndex::Thumb1Y].value, joints[HandJointIndex::Thumb1Z].value);
            GloveVecRes[1] = Vector3Float(joints[HandJointIndex::Thumb2X].value, 0, 0);
            GloveVecRes[2] = Vector3Float(joints[HandJointIndex::Thumb3X].value, 0, 0);
            GloveVecRes[3] = Vector3Float(joints[HandJointIndex::Index1X].value, joints[HandJointIndex::Index1Y].value, joints[HandJointIndex::Index1Z].value);
            GloveVecRes[4] = Vector3Float(joints[HandJointIndex::Index2X].value, 0, 0);
            GloveVecRes[5] = Vector3Float(joints[HandJointIndex::Index3X].value, 0, 0);
            GloveVecRes[6] = Vector3Float(joints[HandJointIndex::Middle1X].value, joints[HandJointIndex::Middle1Y].value, 0);
            GloveVecRes[7] = Vector3Float(joints[HandJointIndex::Middle2X].value, 0, 0);
            GloveVecRes[8] = Vector3Float(joints[HandJointIndex::Middle3X].value, 0, 0);
            GloveVecRes[9] = Vector3Float(joints[HandJointIndex::Ring1X].value, joints[HandJointIndex::Ring1Y].value, 0);
            GloveVecRes[10] = Vector3Float(joints[HandJointIndex::Ring2X].value, 0, 0);
            GloveVecRes[11] = Vector3Float(joints[HandJointIndex::Ring3X].value, 0, 0);
            GloveVecRes[12] = Vector3Float(joints[HandJointIndex::Pinky1X].value, joints[HandJointIndex::Pinky1Y].value, joints[HandJointIndex::Pinky1Z].value);
            GloveVecRes[13] = Vector3Float(joints[HandJointIndex::Pinky2X].value, 0, 0);
            GloveVecRes[14] = Vector3Float(joints[HandJointIndex::Pinky3X].value, 0, 0);

            joints = gloves.handDatas.fingerJoints.fingerJoint_R;

            GloveVecRes[15] = Vector3Float(joints[HandJointIndex::Thumb1X].value, joints[HandJointIndex::Thumb1Y].value, joints[HandJointIndex::Thumb1Z].value);
            GloveVecRes[16] = Vector3Float(joints[HandJointIndex::Thumb2X].value, 0, 0);
            GloveVecRes[17] = Vector3Float(joints[HandJointIndex::Thumb3X].value, 0, 0);
            GloveVecRes[18] = Vector3Float(joints[HandJointIndex::Index1X].value, joints[HandJointIndex::Index1Y].value, joints[HandJointIndex::Index1Z].value);
            GloveVecRes[19] = Vector3Float(joints[HandJointIndex::Index2X].value, 0, 0);
            GloveVecRes[20] = Vector3Float(joints[HandJointIndex::Index3X].value, 0, 0);
            GloveVecRes[21] = Vector3Float(joints[HandJointIndex::Middle1X].value, joints[HandJointIndex::Middle1Y].value, 0);
            GloveVecRes[22] = Vector3Float(joints[HandJointIndex::Middle2X].value, 0, 0);
            GloveVecRes[23] = Vector3Float(joints[HandJointIndex::Middle3X].value, 0, 0);
            GloveVecRes[24] = Vector3Float(joints[HandJointIndex::Ring1X].value, joints[HandJointIndex::Ring1Y].value, 0);
            GloveVecRes[25] = Vector3Float(joints[HandJointIndex::Ring2X].value, 0, 0);
            GloveVecRes[26] = Vector3Float(joints[HandJointIndex::Ring3X].value, 0, 0);
            GloveVecRes[27] = Vector3Float(joints[HandJointIndex::Pinky1X].value, joints[HandJointIndex::Pinky1Y].value, joints[HandJointIndex::Pinky1Z].value);
            GloveVecRes[28] = Vector3Float(joints[HandJointIndex::Pinky2X].value, 0, 0);
            GloveVecRes[29] = Vector3Float(joints[HandJointIndex::Pinky3X].value, 0, 0);

            return GloveVecRes;
        }
    }
    return GloveVecRes;
}

float* UDEGloveSDK::GetVecControllerData(string RoleName)
{
    if(gloveControllerDataList.size() == 0) return ControllerRes;
    for(auto &gloves: gloveControllerDataList)
    {
        if(gloves.roleName == RoleName)
        {
            ControllerRes[0] = gloves.controllerDatas.JoyValue_L[0].value;
            ControllerRes[1] = gloves.controllerDatas.JoyValue_L[1].value;

            ControllerRes[2] = gloves.controllerDatas.ButtonValue_L[0].value;
            ControllerRes[3] = gloves.controllerDatas.ButtonValue_L[1].value;
            ControllerRes[4] = gloves.controllerDatas.ButtonValue_L[2].value;
            ControllerRes[5] = gloves.controllerDatas.ButtonValue_L[3].value;
            
            ControllerRes[6] = gloves.controllerDatas.JoyValue_R[0].value;
            ControllerRes[7] = gloves.controllerDatas.JoyValue_R[1].value;
        
            ControllerRes[8] = gloves.controllerDatas.ButtonValue_R[0].value;
            ControllerRes[9] = gloves.controllerDatas.ButtonValue_R[1].value;
            ControllerRes[10] = gloves.controllerDatas.ButtonValue_R[2].value;
            ControllerRes[11] = gloves.controllerDatas.ButtonValue_R[3].value;

            return ControllerRes;
        }
    }
    return ControllerRes;
}