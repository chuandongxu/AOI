#ifndef DATA_STORE_API_H
#define DATA_STORE_API_H

#ifndef API_CALL
#ifdef  __cplusplus
#define API_CALL(T) __declspec(dllimport) T
#else
#define API_CALL(T) extern __declspec(dllimport) T __stdcall
#endif
#endif

#include "DataStoreStruct.h"

namespace NFG
{
namespace AOI
{
namespace Engine
{

API_CALL(void) GetVersion(String &strVersion);
API_CALL(void) GetErrorDetail(String &errorType, String &errorMessage);
API_CALL(int)  CreateProject(const std::string &strProjFile, String const &userName = String());
API_CALL(int)  OpenProject(const std::string &strProjFile, String const &userName = String());
API_CALL(int)  SetBigBoardCoords(float left, float top, float right, float bottom);
API_CALL(int)  GetBigBoardCoords(float &left, float &top, float &right, float &bottom);
API_CALL(int)  CreateAlignment(Alignment &alignment);
API_CALL(int)  UpdateAlignment(const Alignment &alignment);
API_CALL(int)  GetAllAlignments(AlignmentVector &vecAlignment);
API_CALL(int)  DeleteAlignment(Int64 alignmentId);
API_CALL(int)  CreateBoard(Board &board);
API_CALL(int)  UpdateBoard(const Board &board);
API_CALL(int)  DeleteBoard(Int64 boardId);
API_CALL(int)  GetAllBoards(BoardVector &vecBoard);
API_CALL(int)  CreateDevice(Int64 boardId, Device &device);
API_CALL(int)  CreateDevice(Int64 boardId, DeviceVector &vecDevice);
API_CALL(int)  GetDevice(Int64 deviceId, Device &device);
API_CALL(int)  UpdateDevice(const Device &device);
API_CALL(int)  UpdateDevices(const DeviceVector &vecDevice);
API_CALL(int)  DeleteDevice(Int64 boardId, const String &strDeviceName);
API_CALL(int)  GetBoardDevice(Int64 boardId, DeviceVector &vecDevice);
API_CALL(int)  GetAllDevices(DeviceVector &vecDevice);
API_CALL(int)  CreateWindow(Window &window);
API_CALL(int)  DeleteWindow(Int64 windowId);
API_CALL(int)  UpdateWindow(const Window &window);
API_CALL(int)  GetWindow(Int64 windowId, Window &window);
API_CALL(int)  GetDeviceWindows(Int64 deviceId, WindowVector &vecWindow);
API_CALL(int)  GetDeviceUngroupedWindows(Int64 deviceId, WindowVector &vecWindow);
API_CALL(int)  GetAllWindows(WindowVector &vecWindow);
API_CALL(int)  CreateWindowGroup(WindowGroup &windowGroup);
API_CALL(int)  AddWindowToGroup(Int64 windowId, Int64 groupId);
API_CALL(int)  DeleteWindowGroup(Int64 groupId);
API_CALL(int)  GetAllWindowGroups(Int64Vector &vecGroupId);
API_CALL(int)  GetDeviceWindowGroups(Int64 deviceId, Int64Vector &vecGroupId);
//Get the detail of the windows in a group.
API_CALL(int)  GetGroupWindows(Int64 groupId, WindowGroup &windowGroup);
API_CALL(int)  SetLightSourceCount(Int32 value);
API_CALL(int)  SetLightGroupCount(Int32 value);
API_CALL(int)  SetLights(const LightVector &vecLights);
API_CALL(int)  GetLights(LightVector &vecLights);
API_CALL(int)  AddRecord(Int32 recordId, Binary recordData);
API_CALL(int)  GetRecords(RecordIdDataMap &mapRecordIdData);
}
}
}

#endif