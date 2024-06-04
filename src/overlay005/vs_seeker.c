#include <nitro.h>
#include <string.h>

#include "struct_decls/sys_task.h"
#include "struct_decls/struct_020507E4_decl.h"
#include "struct_decls/struct_020508D4_decl.h"
#include "struct_decls/struct_02061AB4_decl.h"

#include "field/field_system.h"
#include "overlay005/struct_ov5_021F8E3C.h"

#include "unk_02005474.h"
#include "string_template.h"
#include "unk_0200D9E8.h"
#include "heap.h"
#include "unk_0201D15C.h"
#include "unk_0203A378.h"
#include "unk_0203E880.h"
#include "unk_020507CC.h"
#include "unk_020508D4.h"
#include "player_avatar.h"
#include "map_object.h"
#include "unk_020655F4.h"
#include "unk_0206A8DC.h"
#include "unk_0206AFE0.h"
#include "unk_0207D3B8.h"
#include "overlay005/vs_seeker.h"
#include "overlay005/ov5_021DFB54.h"

#define VS_SEEKER_SEARCH_RADIUS_LEFT 7
#define VS_SEEKER_SEARCH_RADIUS_RIGHT 7
#define VS_SEEKER_SEARCH_RADIUS_UP 7
#define VS_SEEKER_SEARCH_RADIUS_DOWN 6

#define VS_SEEKER_MAX_BATTERY 100

#define VS_SEEKER_REMATCH_CHANCE 50

typedef enum VsSeekerUsability {
    VS_SEEKER_USABILITY_NO_BATTERY,
    VS_SEEKER_USABILITY_NO_TRAINERS,
    VS_SEEKER_USABILITY_OK,
} VsSeekerUsability;

typedef enum VsSeekerState {
    VS_SEEKER_STATE_WAIT_FOR_NPCS,
    VS_SEEKER_STATE_CHECK_USABILITY,
    VS_SEEKER_STATE_START,
    VS_SEEKER_STATE_WAIT_FOR_PLAYER_ANIM,
    VS_SEEKER_STATE_PICK_REMATCH_TRAINERS,
    VS_SEEKER_STATE_WAIT_FOR_REMATCH_ANIMS,
    VS_SEEKER_STATE_WAIT_FOR_VS_SEEKER_SFX,
    VS_SEEKER_STATE_NO_BATTERY,
    VS_SEEKER_STATE_NO_TRAINERS,
    VS_SEEKER_STATE_DONE,
} VsSeekerState;

typedef enum VsSeeker2v2TrainerSearchMode {
    VS_SEEKER_2V2_TRAINER_SEARCH_MODE_REMATCH_ANIM_CHECK,
    VS_SEEKER_2V2_TRAINER_SEARCH_MODE_REMATCH_ANIM_SET,
} VsSeeker2v2TrainerSearchMode;

typedef struct {
    u16 unk_00[6];
} RematchData;

typedef struct {
    VsSeekerState state;
    FieldSystem *fieldSystem;
    VarsFlags *events;
    const RematchData *rematchData;
    MapObject *trainers[64];
    u16 numVisibleTrainers;
    u16 numActiveAnimations;
    u16 *result;
    StringTemplate *template;
    SysTask *playerStateTask;
} VsSeekerSystem;

typedef struct {
    SysTask *unk_00;
    SysTask *animationTask;
    VsSeekerSystem *vsSeeker;
} VsSeekerAnimationTask;

void VsSeeker_Start(TaskManager *taskMan, StringTemplate *template, u16 *outResult);
u16 ov5_021DBD98(FieldSystem *fieldSystem, MapObject *param1, u16 param2);
BOOL ov5_021DBB94(FieldSystem *fieldSystem);
static BOOL VsSeeker_IsMoveCodeHidden(u32 param0);
static BOOL TaskManager_ExecuteVsSeeker(TaskManager *taskMan);
static VsSeekerUsability VsSeekerSystem_CheckUsability(VsSeekerSystem *param0);
static void VsSeekerSystem_SetState(VsSeekerSystem *param0, u32 param1);
static void VsSeekerSystem_CollectViableNpcs(VsSeekerSystem *param0);
static void VsSeekerSystem_StartAnimation(VsSeekerSystem *param0, MapObject *param1, const UnkStruct_ov5_021F8E3C *param2);
static void VsSeekerSystem_StartAnimationTask(VsSeekerSystem *param0, SysTask *param1);
static void VsSeeker_TrackAnimation(SysTask *param0, void *param1);
static s32 VsSeekerSystem_GetNumActiveAnimations(VsSeekerSystem *vsSeeker);
static BOOL VsSeekerSystem_PickRematchTrainers(VsSeekerSystem *param0);
static u16 VsSeeker_GetTrainerIDFromMapObject(MapObject *param0);
static void ov5_021DBC08(FieldSystem *fieldSystem);
static u16 ov5_021DBDDC(FieldSystem *fieldSystem, u16 param1);
static u16 ov5_021DBDFC(FieldSystem *fieldSystem, u16 param1);
static u16 ov5_021DBE48(FieldSystem *fieldSystem, u16 param1, u16 param2);
static u16 ov5_021DBE70(u16 param0, u16 param1);
static u16 ov5_021DBEA4(u16 param0, u16 param1);
static BOOL VsSeeker_IsTrainerDoingRematchAnimation(MapObject *param0);
static void VsSeeker_SetTrainerMoveCode(MapObject *param0, u16 param1);
void ov5_021DBED4(FieldSystem *fieldSystem, MapObject *param1);
static BOOL VsSeeker_WaitForNpcsToPause(FieldSystem *fieldSystem);
static MapObject *VsSeeker_GetSecondDoubleBattleTrainer(FieldSystem *fieldSystem, MapObject *trainer, VsSeeker2v2TrainerSearchMode mode);

const RematchData Unk_ov5_021F8E48[] = {
	{ 0xE, 0xE, 0x0, 0x0, 0x0, 0x0 },
	{ 0x15, 0x273, 0x274, 0xffff, 0x275, 0x0 },
	{ 0x2C, 0x2C, 0x0, 0x0, 0x0, 0x0 },
	{ 0x2D, 0xffff, 0x276, 0x277, 0xffff, 0x278 },
	{ 0x14, 0x14, 0x0, 0x0, 0x0, 0x0 },
	{ 0x26, 0xffff, 0x279, 0x27A, 0xffff, 0x27B },
	{ 0x4A, 0xffff, 0xffff, 0x27C, 0xffff, 0x27D },
	{ 0x4C, 0x4C, 0x0, 0x0, 0x0, 0x0 },
	{ 0x51, 0x51, 0x0, 0x0, 0x0, 0x0 },
	{ 0x22A, 0xffff, 0xffff, 0xffff, 0xffff, 0x27E },
	{ 0x22B, 0x22B, 0x0, 0x0, 0x0, 0x0 },
	{ 0x2E, 0xffff, 0x27F, 0x280, 0x281, 0x0 },
	{ 0x38, 0x38, 0x0, 0x0, 0x0, 0x0 },
	{ 0x2F, 0xffff, 0x282, 0x283, 0x284, 0x0 },
	{ 0x39, 0x39, 0x0, 0x0, 0x0, 0x0 },
	{ 0x10, 0x10, 0x0, 0x0, 0x0, 0x0 },
	{ 0x22, 0x22, 0x0, 0x0, 0x0, 0x0 },
	{ 0x179, 0x285, 0x286, 0xffff, 0x287, 0x0 },
	{ 0x84, 0x84, 0x0, 0x0, 0x0, 0x0 },
	{ 0x85, 0x85, 0x0, 0x0, 0x0, 0x0 },
	{ 0x8C, 0xffff, 0xffff, 0xffff, 0x288, 0x0 },
	{ 0x86, 0x86, 0x0, 0x0, 0x0, 0x0 },
	{ 0x87, 0x87, 0x0, 0x0, 0x0, 0x0 },
	{ 0x8D, 0xffff, 0xffff, 0xffff, 0x289, 0x0 },
	{ 0x57, 0x57, 0x0, 0x0, 0x0, 0x0 },
	{ 0x110, 0x110, 0x0, 0x0, 0x0, 0x0 },
	{ 0x111, 0x111, 0x0, 0x0, 0x0, 0x0 },
	{ 0x112, 0xffff, 0xffff, 0xffff, 0x28A, 0x0 },
	{ 0x17D, 0x17D, 0x0, 0x0, 0x0, 0x0 },
	{ 0x17E, 0x17E, 0x0, 0x0, 0x0, 0x0 },
	{ 0x30, 0x30, 0x0, 0x0, 0x0, 0x0 },
	{ 0x3A, 0xffff, 0xffff, 0x28B, 0xffff, 0x28C },
	{ 0x19, 0x19, 0x0, 0x0, 0x0, 0x0 },
	{ 0x1A, 0x1A, 0x0, 0x0, 0x0, 0x0 },
	{ 0x1B, 0xffff, 0x28D, 0x28E, 0xffff, 0x28F },
	{ 0x1C, 0x1C, 0x0, 0x0, 0x0, 0x0 },
	{ 0x1D, 0x1D, 0x0, 0x0, 0x0, 0x0 },
	{ 0x1E, 0x1E, 0x0, 0x0, 0x0, 0x0 },
	{ 0x1F, 0xffff, 0x290, 0x291, 0xffff, 0x292 },
	{ 0x20, 0x20, 0x0, 0x0, 0x0, 0x0 },
	{ 0x4B, 0x4B, 0x0, 0x0, 0x0, 0x0 },
	{ 0x49, 0xffff, 0xffff, 0x293, 0xffff, 0x294 },
	{ 0x115, 0xffff, 0xffff, 0xffff, 0xffff, 0x295 },
	{ 0x22D, 0xffff, 0xffff, 0xffff, 0xffff, 0x296 },
	{ 0x22E, 0x22E, 0x0, 0x0, 0x0, 0x0 },
	{ 0x22F, 0x22F, 0x0, 0x0, 0x0, 0x0 },
	{ 0x230, 0x230, 0x0, 0x0, 0x0, 0x0 },
	{ 0x42, 0x42, 0x0, 0x0, 0x0, 0x0 },
	{ 0xAA, 0xffff, 0xffff, 0xffff, 0x297, 0x0 },
	{ 0x116, 0xffff, 0xffff, 0x298, 0xffff, 0x299 },
	{ 0x11A, 0x11A, 0x0, 0x0, 0x0, 0x0 },
	{ 0x233, 0xffff, 0xffff, 0xffff, 0xffff, 0x29A },
	{ 0x234, 0xffff, 0xffff, 0xffff, 0xffff, 0x29B },
	{ 0x235, 0x235, 0x0, 0x0, 0x0, 0x0 },
	{ 0x236, 0x236, 0x0, 0x0, 0x0, 0x0 },
	{ 0x237, 0x237, 0x0, 0x0, 0x0, 0x0 },
	{ 0x238, 0x238, 0x0, 0x0, 0x0, 0x0 },
	{ 0x43, 0x43, 0x0, 0x0, 0x0, 0x0 },
	{ 0xAB, 0xffff, 0xffff, 0xffff, 0x29C, 0x0 },
	{ 0x11E, 0x11E, 0x0, 0x0, 0x0, 0x0 },
	{ 0x11F, 0xffff, 0xffff, 0x29D, 0xffff, 0x29E },
	{ 0x23D, 0xffff, 0xffff, 0xffff, 0xffff, 0x29F },
	{ 0x23E, 0xffff, 0xffff, 0xffff, 0xffff, 0x2A0 },
	{ 0x23F, 0x23F, 0x0, 0x0, 0x0, 0x0 },
	{ 0x240, 0x240, 0x0, 0x0, 0x0, 0x0 },
	{ 0x241, 0x241, 0x0, 0x0, 0x0, 0x0 },
	{ 0x242, 0x242, 0x0, 0x0, 0x0, 0x0 },
	{ 0x77, 0xffff, 0xffff, 0x2A1, 0xffff, 0x2A2 },
	{ 0x120, 0xffff, 0xffff, 0xffff, 0xffff, 0x2A3 },
	{ 0x247, 0xffff, 0xffff, 0xffff, 0xffff, 0x2A4 },
	{ 0x78, 0xffff, 0xffff, 0x2A5, 0xffff, 0x2A6 },
	{ 0x121, 0xffff, 0xffff, 0xffff, 0xffff, 0x2A7 },
	{ 0x249, 0xffff, 0xffff, 0xffff, 0xffff, 0x2A8 },
	{ 0x122, 0x122, 0x0, 0x0, 0x0, 0x0 },
	{ 0x123, 0x123, 0x0, 0x0, 0x0, 0x0 },
	{ 0x124, 0xffff, 0xffff, 0x2A9, 0xffff, 0x2AA },
	{ 0x16, 0x16, 0x0, 0x0, 0x0, 0x0 },
	{ 0x17, 0xffff, 0x2AB, 0xffff, 0x2AC, 0x0 },
	{ 0x18, 0x18, 0x0, 0x0, 0x0, 0x0 },
	{ 0x2B, 0x2B, 0x0, 0x0, 0x0, 0x0 },
	{ 0x5B, 0x5B, 0x0, 0x0, 0x0, 0x0 },
	{ 0x5C, 0x5C, 0x0, 0x0, 0x0, 0x0 },
	{ 0x5D, 0x5D, 0x0, 0x0, 0x0, 0x0 },
	{ 0x6F, 0x6F, 0x0, 0x0, 0x0, 0x0 },
	{ 0x99, 0xffff, 0xffff, 0xffff, 0x2AD, 0x0 },
	{ 0x9A, 0x9A, 0x0, 0x0, 0x0, 0x0 },
	{ 0xA8, 0xA8, 0x0, 0x0, 0x0, 0x0 },
	{ 0xAC, 0xAC, 0x0, 0x0, 0x0, 0x0 },
	{ 0xAD, 0xffff, 0xffff, 0xffff, 0x2AE, 0x0 },
	{ 0xAE, 0xAE, 0x0, 0x0, 0x0, 0x0 },
	{ 0xAF, 0xAF, 0x0, 0x0, 0x0, 0x0 },
	{ 0xF, 0xF, 0x0, 0x0, 0x0, 0x0 },
	{ 0x41, 0xffff, 0xffff, 0x2AF, 0xffff, 0x2B0 },
	{ 0x126, 0x126, 0x0, 0x0, 0x0, 0x0 },
	{ 0x79, 0xffff, 0xffff, 0x2B1, 0xffff, 0x2B2 },
	{ 0x54, 0xffff, 0x2B3, 0x2B4, 0xffff, 0x2B5 },
	{ 0x12C, 0xffff, 0xffff, 0xffff, 0x2B6, 0x0 },
	{ 0x71, 0x71, 0x0, 0x0, 0x0, 0x0 },
	{ 0x72, 0x72, 0x0, 0x0, 0x0, 0x0 },
	{ 0x130, 0x130, 0x0, 0x0, 0x0, 0x0 },
	{ 0x131, 0x131, 0x0, 0x0, 0x0, 0x0 },
	{ 0x1B9, 0x1B9, 0x0, 0x0, 0x0, 0x0 },
	{ 0x1BA, 0xffff, 0xffff, 0x2B7, 0x2B8, 0x0 },
	{ 0x31, 0x31, 0x0, 0x0, 0x0, 0x0 },
	{ 0x82, 0x82, 0x0, 0x0, 0x0, 0x0 },
	{ 0x132, 0x132, 0x0, 0x0, 0x0, 0x0 },
	{ 0x133, 0xffff, 0xffff, 0x2B9, 0xffff, 0x2BA },
	{ 0x134, 0x134, 0x0, 0x0, 0x0, 0x0 },
	{ 0x2A, 0x2A, 0x0, 0x0, 0x0, 0x0 },
	{ 0x45, 0x45, 0x0, 0x0, 0x0, 0x0 },
	{ 0x4F, 0x4F, 0x0, 0x0, 0x0, 0x0 },
	{ 0x7E, 0x7E, 0x0, 0x0, 0x0, 0x0 },
	{ 0x7F, 0x7F, 0x0, 0x0, 0x0, 0x0 },
	{ 0x80, 0x80, 0x0, 0x0, 0x0, 0x0 },
	{ 0x81, 0x81, 0x0, 0x0, 0x0, 0x0 },
	{ 0x94, 0x94, 0x0, 0x0, 0x0, 0x0 },
	{ 0x102, 0xffff, 0xffff, 0xffff, 0x2BB, 0x0 },
	{ 0x139, 0x139, 0x0, 0x0, 0x0, 0x0 },
	{ 0x24B, 0xffff, 0xffff, 0xffff, 0xffff, 0x2BC },
	{ 0x24C, 0x24C, 0x0, 0x0, 0x0, 0x0 },
	{ 0x37, 0x37, 0x0, 0x0, 0x0, 0x0 },
	{ 0x55, 0xffff, 0xffff, 0x2BD, 0xffff, 0x2BE },
	{ 0x3, 0x3, 0x0, 0x0, 0x0, 0x0 },
	{ 0xB, 0x2BF, 0x2C0, 0xffff, 0x2C1, 0x0 },
	{ 0xC, 0xC, 0x0, 0x0, 0x0, 0x0 },
	{ 0x142, 0x2C2, 0x2C3, 0x2C4, 0x0, 0x0 },
	{ 0x143, 0x143, 0x0, 0x0, 0x0, 0x0 },
	{ 0x12, 0x12, 0x0, 0x0, 0x0, 0x0 },
	{ 0x13, 0x13, 0x0, 0x0, 0x0, 0x0 },
	{ 0x24, 0x24, 0x0, 0x0, 0x0, 0x0 },
	{ 0x25, 0x25, 0x0, 0x0, 0x0, 0x0 },
	{ 0x27, 0x27, 0x0, 0x0, 0x0, 0x0 },
	{ 0x28, 0x28, 0x0, 0x0, 0x0, 0x0 },
	{ 0x29, 0x29, 0x0, 0x0, 0x0, 0x0 },
	{ 0x146, 0x146, 0x0, 0x0, 0x0, 0x0 },
	{ 0x1C3, 0xffff, 0x2C5, 0x2C6, 0xffff, 0x2C7 },
	{ 0xD, 0x2C8, 0x2C9, 0xffff, 0x2CA, 0x0 },
	{ 0x147, 0xffff, 0x2CB, 0x2CC, 0x2CD, 0x0 },
	{ 0x148, 0x148, 0x0, 0x0, 0x0, 0x0 },
	{ 0x11, 0x11, 0x0, 0x0, 0x0, 0x0 },
	{ 0x23, 0x23, 0x0, 0x0, 0x0, 0x0 },
	{ 0x1C8, 0x2CE, 0x2CF, 0xffff, 0x2D0, 0x0 },
	{ 0x35, 0xffff, 0x2D1, 0x2D2, 0x2D3, 0x0 },
	{ 0x58, 0x58, 0x0, 0x0, 0x0, 0x0 },
	{ 0x59, 0x59, 0x0, 0x0, 0x0, 0x0 },
	{ 0x5A, 0x5A, 0x0, 0x0, 0x0, 0x0 },
	{ 0x66, 0x66, 0x0, 0x0, 0x0, 0x0 },
	{ 0x14C, 0xffff, 0x2D4, 0x2D5, 0x2D6, 0x0 },
	{ 0x14D, 0x14D, 0x0, 0x0, 0x0, 0x0 },
	{ 0x52, 0x52, 0x0, 0x0, 0x0, 0x0 },
	{ 0x14F, 0xffff, 0xffff, 0xffff, 0x2D7, 0x2D8 },
	{ 0x53, 0xffff, 0x2D9, 0x2DA, 0xffff, 0x2DB },
	{ 0x5E, 0xffff, 0x2DC, 0x2DD, 0xffff, 0x2DE },
	{ 0x5F, 0x5F, 0x0, 0x0, 0x0, 0x0 },
	{ 0x24F, 0xffff, 0xffff, 0xffff, 0xffff, 0x2DF },
	{ 0x250, 0x250, 0x0, 0x0, 0x0, 0x0 },
	{ 0x251, 0x251, 0x0, 0x0, 0x0, 0x0 },
	{ 0x60, 0xffff, 0x2E0, 0x2E1, 0xffff, 0x2E2 },
	{ 0x61, 0x61, 0x0, 0x0, 0x0, 0x0 },
	{ 0x252, 0xffff, 0xffff, 0xffff, 0xffff, 0x2E3 },
	{ 0x253, 0x253, 0x0, 0x0, 0x0, 0x0 },
	{ 0x254, 0x254, 0x0, 0x0, 0x0, 0x0 },
	{ 0x6E, 0x6E, 0x0, 0x0, 0x0, 0x0 },
	{ 0xB4, 0xB4, 0x0, 0x0, 0x0, 0x0 },
	{ 0x151, 0xffff, 0xffff, 0xffff, 0x2E4, 0x2E5 },
	{ 0x152, 0x152, 0x0, 0x0, 0x0, 0x0 },
	{ 0x1E2, 0xffff, 0xffff, 0xffff, 0xffff, 0x2E6 },
	{ 0x62, 0x62, 0x0, 0x0, 0x0, 0x0 },
	{ 0x63, 0x63, 0x0, 0x0, 0x0, 0x0 },
	{ 0x1E6, 0xffff, 0x2E7, 0x2E8, 0x2E9, 0x0 },
	{ 0x46, 0x46, 0x0, 0x0, 0x0, 0x0 },
	{ 0x47, 0x47, 0x0, 0x0, 0x0, 0x0 },
	{ 0x48, 0x48, 0x0, 0x0, 0x0, 0x0 },
	{ 0x4E, 0xffff, 0x2EA, 0x2EB, 0x2EC, 0x0 },
	{ 0x50, 0x50, 0x0, 0x0, 0x0, 0x0 },
	{ 0x92, 0x92, 0x0, 0x0, 0x0, 0x0 },
	{ 0x93, 0x93, 0x0, 0x0, 0x0, 0x0 },
	{ 0x1E8, 0x1E8, 0x0, 0x0, 0x0, 0x0 },
	{ 0x1E9, 0x1E9, 0x0, 0x0, 0x0, 0x0 },
	{ 0x1EA, 0x1EA, 0x0, 0x0, 0x0, 0x0 },
	{ 0x15A, 0xffff, 0xffff, 0x2ED, 0xffff, 0x2EE },
	{ 0x1EB, 0x1EB, 0x0, 0x0, 0x0, 0x0 },
	{ 0x1EC, 0x1EC, 0x0, 0x0, 0x0, 0x0 },
	{ 0x88, 0x88, 0x0, 0x0, 0x0, 0x0 },
	{ 0x89, 0xffff, 0xffff, 0x2EF, 0x2F0, 0x0 },
	{ 0x8E, 0x8E, 0x0, 0x0, 0x0, 0x0 },
	{ 0x8F, 0xffff, 0xffff, 0x2F1, 0xffff, 0x2F2 },
	{ 0x8A, 0x8A, 0x0, 0x0, 0x0, 0x0 },
	{ 0x8B, 0xffff, 0xffff, 0x2F3, 0x2F4, 0x0 },
	{ 0x90, 0x90, 0x0, 0x0, 0x0, 0x0 },
	{ 0x91, 0xffff, 0xffff, 0x2F5, 0x2F6, 0x0 },
	{ 0x67, 0x67, 0x0, 0x0, 0x0, 0x0 },
	{ 0x68, 0x68, 0x0, 0x0, 0x0, 0x0 },
	{ 0x9F, 0x9F, 0x0, 0x0, 0x0, 0x0 },
	{ 0xA0, 0xA0, 0x0, 0x0, 0x0, 0x0 },
	{ 0xA1, 0xA1, 0x0, 0x0, 0x0, 0x0 },
	{ 0xA6, 0xA6, 0x0, 0x0, 0x0, 0x0 },
	{ 0xB7, 0xffff, 0xffff, 0xffff, 0xffff, 0x2F7 },
	{ 0xB8, 0xB8, 0x0, 0x0, 0x0, 0x0 },
	{ 0xB9, 0xffff, 0xffff, 0xffff, 0xffff, 0x2F8 },
	{ 0xBA, 0xBA, 0x0, 0x0, 0x0, 0x0 },
	{ 0xBB, 0xBB, 0x0, 0x0, 0x0, 0x0 },
	{ 0xBC, 0xBC, 0x0, 0x0, 0x0, 0x0 },
	{ 0x255, 0xffff, 0xffff, 0xffff, 0xffff, 0x2F9 },
	{ 0x256, 0x256, 0x0, 0x0, 0x0, 0x0 },
	{ 0x257, 0x257, 0x0, 0x0, 0x0, 0x0 },
	{ 0x258, 0x258, 0x0, 0x0, 0x0, 0x0 },
	{ 0x69, 0x69, 0x0, 0x0, 0x0, 0x0 },
	{ 0x6A, 0x6A, 0x0, 0x0, 0x0, 0x0 },
	{ 0xA2, 0xA2, 0x0, 0x0, 0x0, 0x0 },
	{ 0xA3, 0xA3, 0x0, 0x0, 0x0, 0x0 },
	{ 0xA4, 0xA4, 0x0, 0x0, 0x0, 0x0 },
	{ 0xA7, 0xA7, 0x0, 0x0, 0x0, 0x0 },
	{ 0xBD, 0xffff, 0xffff, 0xffff, 0xffff, 0x2FA },
	{ 0xBE, 0xBE, 0x0, 0x0, 0x0, 0x0 },
	{ 0xBF, 0xBF, 0x0, 0x0, 0x0, 0x0 },
	{ 0xC0, 0xffff, 0xffff, 0xffff, 0xffff, 0x2FB },
	{ 0xC1, 0xC1, 0x0, 0x0, 0x0, 0x0 },
	{ 0xC2, 0xC2, 0x0, 0x0, 0x0, 0x0 },
	{ 0x1EE, 0x1EE, 0x0, 0x0, 0x0, 0x0 },
	{ 0x259, 0xffff, 0xffff, 0xffff, 0xffff, 0x2FC },
	{ 0x25A, 0x25A, 0x0, 0x0, 0x0, 0x0 },
	{ 0x25B, 0x25B, 0x0, 0x0, 0x0, 0x0 },
	{ 0x25C, 0x25C, 0x0, 0x0, 0x0, 0x0 },
	{ 0x1, 0x2FD, 0x2FE, 0x2FF, 0x0, 0x0 },
	{ 0x2, 0x2, 0x0, 0x0, 0x0, 0x0 },
	{ 0x4, 0x4, 0x0, 0x0, 0x0, 0x0 },
	{ 0xA, 0xA, 0x0, 0x0, 0x0, 0x0 },
	{ 0x21, 0x21, 0x0, 0x0, 0x0, 0x0 },
	{ 0x163, 0x300, 0x301, 0xffff, 0x302, 0x0 },
	{ 0x164, 0x164, 0x0, 0x0, 0x0, 0x0 },
	{ 0x6B, 0xffff, 0x303, 0xffff, 0x304, 0x0 },
	{ 0x166, 0x166, 0x0, 0x0, 0x0, 0x0 },
	{ 0x167, 0x167, 0x0, 0x0, 0x0, 0x0 },
	{ 0x6C, 0xffff, 0xffff, 0x305, 0x307, 0x0 },
	{ 0x168, 0x168, 0x0, 0x0, 0x0, 0x0 },
	{ 0x169, 0x169, 0x0, 0x0, 0x0, 0x0 },
	{ 0x44, 0xffff, 0xffff, 0x306, 0xffff, 0x308 },
	{ 0x16A, 0xffff, 0xffff, 0xffff, 0xffff, 0x309 },
	{ 0x25D, 0xffff, 0xffff, 0xffff, 0xffff, 0x30A }
};

static const UnkStruct_ov5_021F8E3C Unk_ov5_021F8E3C[] = {
    {0x1, 0x1},
    {0x67, 0x1},
    {0xfe, 0x0}
};

static const UnkStruct_ov5_021F8E3C Unk_ov5_021F8E34[] = {
    {0x4B, 0x1},
    {0xfe, 0x0}
};

void VsSeeker_Start(TaskManager *taskMan, StringTemplate *template, u16 *outResult)
{
    FieldSystem *fieldSystem = TaskManager_FieldSystem(taskMan);
    VsSeekerSystem *vsSeeker = Heap_AllocFromHeap(4, sizeof(VsSeekerSystem));

    if (vsSeeker == NULL) {
        GF_ASSERT(FALSE);
        return;
    }

    memset(vsSeeker, 0, sizeof(VsSeekerSystem));

    vsSeeker->fieldSystem = fieldSystem;
    vsSeeker->events = SaveData_GetVarsFlags(fieldSystem->saveData);
    vsSeeker->result = outResult;
    vsSeeker->template = template;

    FieldTask_Start(taskMan, TaskManager_ExecuteVsSeeker, vsSeeker);
    return;
}

static BOOL TaskManager_ExecuteVsSeeker(TaskManager *taskMan)
{
    s32 missingBattery, numDigits;
    VsSeekerUsability usability;
    VsSeekerSystem *vsSeeker = TaskManager_Environment(taskMan);

    switch (vsSeeker->state) {
    case VS_SEEKER_STATE_WAIT_FOR_NPCS:
        if (VsSeeker_WaitForNpcsToPause(vsSeeker->fieldSystem) == TRUE) {
            VsSeekerSystem_SetState(vsSeeker, VS_SEEKER_STATE_CHECK_USABILITY);
        }
        break;
    case VS_SEEKER_STATE_CHECK_USABILITY:
        VsSeekerSystem_CollectViableNpcs(vsSeeker);
        usability = VsSeekerSystem_CheckUsability(vsSeeker);

        if (usability == VS_SEEKER_USABILITY_OK) {
            *vsSeeker->result = 0;
            VsSeekerSystem_SetState(vsSeeker, VS_SEEKER_STATE_START);
        } else if (usability == VS_SEEKER_USABILITY_NO_BATTERY) {
            *vsSeeker->result = 1;
            VsSeekerSystem_SetState(vsSeeker, 7);
        } else { // VS_SEEKER_USABILITY_NO_TRAINERS
            *vsSeeker->result = 2;
            VsSeekerSystem_SetState(vsSeeker, 8);
        }
        break;
    case VS_SEEKER_STATE_START:
        vsSeeker->playerStateTask = Player_SetStateVsSeeker(vsSeeker->fieldSystem);
        Sound_PlayEffect(1568);
        Events_SetVsSeekerBattery(vsSeeker->events, 0);
        VsSeekerSystem_SetState(vsSeeker, VS_SEEKER_STATE_WAIT_FOR_PLAYER_ANIM);
        break;
    case VS_SEEKER_STATE_WAIT_FOR_PLAYER_ANIM:
        if (VsSeekerSystem_GetNumActiveAnimations(vsSeeker) == 0) {
            VsSeekerSystem_SetState(vsSeeker, VS_SEEKER_STATE_PICK_REMATCH_TRAINERS);
        }
        break;
    case VS_SEEKER_STATE_PICK_REMATCH_TRAINERS:
        if (VsSeekerSystem_PickRematchTrainers(vsSeeker) == FALSE) {
            *vsSeeker->result = 3;
        }

        VsSeekerSystem_SetState(vsSeeker, VS_SEEKER_STATE_WAIT_FOR_REMATCH_ANIMS);
        break;
    case VS_SEEKER_STATE_WAIT_FOR_REMATCH_ANIMS:
        if (VsSeekerSystem_GetNumActiveAnimations(vsSeeker) == 0) {
            VsSeekerSystem_SetState(vsSeeker, VS_SEEKER_STATE_WAIT_FOR_VS_SEEKER_SFX);
        }
        break;
    case VS_SEEKER_STATE_WAIT_FOR_VS_SEEKER_SFX:
        if (Sound_IsEffectPlaying(1568) == FALSE) {
            Player_ResetVsSeekerState(vsSeeker->playerStateTask);
            VsSeekerSystem_SetState(vsSeeker, 9);
        }
        break;
    case VS_SEEKER_STATE_NO_BATTERY:
        missingBattery = (VS_SEEKER_MAX_BATTERY - Events_GetVsSeekerBattery(vsSeeker->events));

        if ((missingBattery / 10) == 0) {
            numDigits = 1;
        } else if ((missingBattery / 100) == 0) {
            numDigits = 2;
        } else {
            numDigits = 3;
        }

        StringTemplate_SetNumber(vsSeeker->template, 0, missingBattery, numDigits, 1, 1);
        VsSeekerSystem_SetState(vsSeeker, 9);
        break;
    case VS_SEEKER_STATE_NO_TRAINERS:
        VsSeekerSystem_SetState(vsSeeker, 9);
        break;
    case VS_SEEKER_STATE_DONE:
        Heap_FreeToHeap(vsSeeker);
        return TRUE;
    }

    return FALSE;
}

static void VsSeekerSystem_SetState (VsSeekerSystem *vsSeeker, u32 state)
{
    vsSeeker->state = state;
    return;
}

static VsSeekerUsability VsSeekerSystem_CheckUsability(VsSeekerSystem *param0)
{
    if (Events_GetVsSeekerBattery(param0->events) == VS_SEEKER_MAX_BATTERY) {
        if (param0->numVisibleTrainers == 0) {
            return 1;
        }

        return 2;
    }

    return 0;
}

static void VsSeekerSystem_CollectViableNpcs(VsSeekerSystem *vsSeeker)
{
    int trainerX, trainerZ;
    int playerX, playerZ, numVisibleTrainers;
    int xMin, xMax, zMin, zMax;
    u32 npcCount = FieldSystem_GetNpcCount(vsSeeker->fieldSystem);

    numVisibleTrainers = 0;

    for (int i = 0; i < npcCount; i++) {
        vsSeeker->trainers[i] = NULL;
    }

    playerX = Player_GetXPos(vsSeeker->fieldSystem->playerAvatar);
    playerZ = Player_GetZPos(vsSeeker->fieldSystem->playerAvatar);
    xMin = playerX - VS_SEEKER_SEARCH_RADIUS_LEFT;
    xMax = playerX + VS_SEEKER_SEARCH_RADIUS_RIGHT;
    zMin = playerZ - VS_SEEKER_SEARCH_RADIUS_UP;
    zMax = playerZ + VS_SEEKER_SEARCH_RADIUS_DOWN;

    if (xMin < 0) {
        xMin = 0;
    }

    if (zMin < 0) {
        zMin = 0;
    }

    for (int i = 0; i < npcCount; i++) {
        MapObject *mapObj = MapObjMan_LocalMapObjByIndex(vsSeeker->fieldSystem->mapObjMan, i);

        if (mapObj == NULL) {
            continue;
        }

        u32 eventType = MapObject_GetEventType(mapObj);

        switch (eventType) {
        case 0x1:
        case 0x2:
        case 0x4:
        case 0x5:
        case 0x6:
        case 0x7:
        case 0x8:
            trainerX = MapObject_GetXPos(mapObj);
            trainerZ = MapObject_GetZPos(mapObj);

            if ((trainerX >= xMin) && (trainerX <= xMax) && (trainerZ >= zMin) && (trainerZ <= zMax)) {
                if (VsSeeker_IsMoveCodeHidden(MapObject_GetMoveCode(mapObj)) == FALSE) {
                    vsSeeker->trainers[numVisibleTrainers] = mapObj;
                    numVisibleTrainers++;
                }
            }
        }
    }

    vsSeeker->numVisibleTrainers = numVisibleTrainers;
    return;
}

static BOOL VsSeeker_IsMoveCodeHidden(u32 moveCode)
{
    switch (moveCode) {
    case 0x33:
    case 0x34:
    case 0x35:
    case 0x36:
        return TRUE;
    }

    return FALSE;
}

BOOL ov5_021DBB94 (FieldSystem *fieldSystem)
{
    VarsFlags *v0 = SaveData_GetVarsFlags(fieldSystem->saveData);
    u16 v1 = Events_GetVsSeekerBattery(v0);
    u16 v2 = sub_0206B108(v0);

    if (sub_0207D688(sub_0207D990(fieldSystem->saveData), 443, 1, 4) == 1) {
        if (v1 < 100) {
            v1++;
            Events_SetVsSeekerBattery(v0, v1);
        }
    }

    if (sub_0206A9E4(v0) == 1) {
        if (v2 < 100) {
            v2++;
            sub_0206B118(v0, v2);
        }

        if (v2 == 100) {
            FieldEvents_ResetVSSeeker(v0);
            ov5_021DBC08(fieldSystem);
        }
    }

    return 0;
}

static void ov5_021DBC08 (FieldSystem *fieldSystem)
{
    int v0;
    MapObject *v1;
    u32 v2 = FieldSystem_GetNpcCount(fieldSystem);

    for (v0 = 0; v0 < v2; v0++) {
        v1 = MapObjMan_LocalMapObjByIndex(fieldSystem->mapObjMan, v0);

        if (v1 == NULL) {
            continue;
        }

        if (MapObject_GetMoveCode(v1) == 0x31) {
            VsSeeker_SetTrainerMoveCode(v1, 0x2);
        }
    }

    return;
}

static void VsSeekerSystem_StartAnimation(VsSeekerSystem *vsSeeker, MapObject *mapObj, const UnkStruct_ov5_021F8E3C *animCmdList)
{
    SysTask *animTask = MapObject_StartAnimation(mapObj, animCmdList);
    vsSeeker->numActiveAnimations++;

    VsSeekerSystem_StartAnimationTask(vsSeeker, animTask);
    return;
}

static void VsSeekerSystem_StartAnimationTask(VsSeekerSystem *vsSeeker, SysTask *animTask)
{
    VsSeekerAnimationTask *vssAnimTask = NULL;

    vssAnimTask = Heap_AllocFromHeap(4, sizeof(VsSeekerAnimationTask));

    if (vssAnimTask == NULL) {
        GF_ASSERT(FALSE);
        return;
    }

    vssAnimTask->vsSeeker = vsSeeker;
    vssAnimTask->animationTask = animTask;
    vssAnimTask->unk_00 = SysTask_Start(VsSeeker_TrackAnimation, vssAnimTask, 0);

    return;
}

static void VsSeeker_TrackAnimation(SysTask *task, void *param)
{
    VsSeekerAnimationTask *vssAnimTask = (VsSeekerAnimationTask *)param;

    if (MapObject_HasAnimationEnded(vssAnimTask->animationTask) == TRUE) {
        if (vssAnimTask->vsSeeker->numActiveAnimations == 0) {
            GF_ASSERT(FALSE);
        }

        vssAnimTask->vsSeeker->numActiveAnimations--;

        MapObject_FinishAnimation(vssAnimTask->animationTask);
        SysTask_Done(vssAnimTask->unk_00);
        Heap_FreeToHeapExplicit(4, param);
    }

    return;
}

static s32 VsSeekerSystem_GetNumActiveAnimations(VsSeekerSystem *vsSeeker)
{
    return vsSeeker->numActiveAnimations;
}

static BOOL VsSeekerSystem_PickRematchTrainers(VsSeekerSystem *vsSeeker)
{
    VarsFlags *events = SaveData_GetVarsFlags(vsSeeker->fieldSystem->saveData);
    MapObject *secondTrainer;
    u16 trainerID;
    int i;

    BOOL anyAvailable = FALSE;

    for (i = 0; i < vsSeeker->numVisibleTrainers; i++) {
        trainerID = VsSeeker_GetTrainerIDFromMapObject(vsSeeker->trainers[i]);

        if (Script_HasBeatenTrainer(vsSeeker->fieldSystem, trainerID) == FALSE) {
            VsSeekerSystem_StartAnimation(vsSeeker, vsSeeker->trainers[i], Unk_ov5_021F8E34);
            anyAvailable = TRUE;
        } else {
            if (((LCRNG_Next() % 100) < VS_SEEKER_REMATCH_CHANCE) && 
                (VsSeeker_IsTrainerDoingRematchAnimation(vsSeeker->trainers[i]) == FALSE)) {
                VsSeeker_SetTrainerMoveCode(vsSeeker->trainers[i], 0x31);
                VsSeekerSystem_StartAnimation(vsSeeker, vsSeeker->trainers[i], Unk_ov5_021F8E3C);

                secondTrainer = VsSeeker_GetSecondDoubleBattleTrainer(vsSeeker->fieldSystem, vsSeeker->trainers[i], VS_SEEKER_2V2_TRAINER_SEARCH_MODE_REMATCH_ANIM_CHECK);

                if (secondTrainer != NULL) {
                    VsSeeker_SetTrainerMoveCode(secondTrainer, 0x31);
                    VsSeekerSystem_StartAnimation(vsSeeker, secondTrainer, Unk_ov5_021F8E3C);
                }

                anyAvailable = TRUE;
                Events_SetVsSeekerUsedFlag(events);
            } else {
                (void)0;
            }
        }
    }

    return anyAvailable;
}

static u16 VsSeeker_GetTrainerIDFromMapObject(MapObject *mapObj)
{
    u32 eventID = MapObject_GetEventID(mapObj);
    return Script_GetTrainerIDFromEventID(eventID);
}

u16 ov5_021DBD98 (FieldSystem *fieldSystem, MapObject *param1, u16 param2)
{
    u16 v0, v1, v2;

    if (VsSeeker_IsTrainerDoingRematchAnimation(param1) == 0) {
        return 0;
    }

    v0 = ov5_021DBDDC(fieldSystem, param2);

    if (v0 == 0xff) {
        return 0;
    }

    v1 = ov5_021DBDFC(fieldSystem, v0);
    v1 = ov5_021DBE48(fieldSystem, v0, v1);
    v2 = ov5_021DBEA4(v0, v1);

    return v2;
}

static u16 ov5_021DBDDC (FieldSystem *fieldSystem, u16 param1)
{
    int v0, v1;
    const RematchData *v2 = Unk_ov5_021F8E48;

    for (v0 = 0; v0 < (NELEMS(Unk_ov5_021F8E48)); v0++) {
        if (v2[v0].unk_00[0] != param1) {
            continue;
        }

        return v0;
    }

    return 0xff;
}

static u16 ov5_021DBDFC (FieldSystem *fieldSystem, u16 param1)
{
    int v0, v1;
    const RematchData *v2 = Unk_ov5_021F8E48;

    for (v1 = 1; v1 < 6; v1++) {
        if (v2[param1].unk_00[v1] == 0) {
            return v1 - 1;
        }

        if (v2[param1].unk_00[v1] != 0xffff) {
            if (Script_HasBeatenTrainer(fieldSystem, v2[param1].unk_00[v1]) == 0) {
                return v1;
            }
        }
    }

    return v1 - 1;
}

static u16 ov5_021DBE48 (FieldSystem *fieldSystem, u16 param1, u16 param2)
{
    VarsFlags * v0 = SaveData_GetVarsFlags(fieldSystem->saveData);
    u16 v1 = param2;

    switch (param2) {
    case 0:
        break;
    default:
        if (sub_0206AB00(v0, param2) == 0) {
            v1 = ov5_021DBE70(param1, param2);
        }
        break;
    }

    return v1;
}

static u16 ov5_021DBE70 (u16 param0, u16 param1)
{
    u16 v0;
    const RematchData *v1 = Unk_ov5_021F8E48;

    for (v0 = (param1 - 1); v0 > 0; v0--) {
        if (v1[param0].unk_00[v0] != 0xffff) {
            return v0;
        }
    }

    return 0;
}

static u16 ov5_021DBEA4 (u16 param0, u16 param1)
{
    const RematchData *v0 = Unk_ov5_021F8E48;

    return v0[param0].unk_00[param1];
}

static BOOL VsSeeker_IsTrainerDoingRematchAnimation(MapObject *param0)
{
    if (MapObject_GetMoveCode(param0) == 0x31) {
        return 1;
    }

    return 0;
}

static void VsSeeker_SetTrainerMoveCode (MapObject *param0, u16 param1)
{
    sub_020633A8(param0, param1);
    return;
}

void ov5_021DBED4 (FieldSystem *fieldSystem, MapObject *param1)
{
    MapObject *v0;
    u32 v1;
    int v2;

    if (param1 == NULL) {
        return;
    }

    v2 = MapObject_Dir(param1);

    if (v2 == 0) {
        v1 = 0xe;
    } else if (v2 == 1) {
        v1 = 0xf;
    } else if (v2 == 2) {
        v1 = 0x10;
    } else {
        v1 = 0x11;
    }

    v0 = VsSeeker_GetSecondDoubleBattleTrainer(fieldSystem, param1, 1);

    if (v0 != NULL) {
        VsSeeker_SetTrainerMoveCode(v0, v1);
    }

    VsSeeker_SetTrainerMoveCode(param1, v1);
    return;
}

static BOOL VsSeeker_WaitForNpcsToPause(FieldSystem *fieldSystem)
{
    u32 npcCount = FieldSystem_GetNpcCount(fieldSystem);

    BOOL anyMoving = FALSE;

    for (int i = 0; i < npcCount; i++) {
        MapObject *mapObj = MapObjMan_LocalMapObjByIndex(fieldSystem->mapObjMan, i);

        if (mapObj == NULL) {
            continue;
        }

        if (MapObject_IsMoving(mapObj) == TRUE) {
            sub_02062DDC(mapObj);
            anyMoving = TRUE;
        } else {
            sub_02062DD0(mapObj);
        }
    }

    if (anyMoving == FALSE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static MapObject *VsSeeker_GetSecondDoubleBattleTrainer(FieldSystem *fieldSystem, MapObject *trainer, VsSeeker2v2TrainerSearchMode mode)
{
    MapObject *mapObj;
    u32 i, eventType, secondTrainerEventID, secondTrainerID;
    u32 npcCount = FieldSystem_GetNpcCount(fieldSystem);
    u16 eventID = MapObject_GetEventID(trainer);
    u16 trainerID = Script_GetTrainerIDFromEventID(eventID);

    if (Script_IsDoubleBattle(trainerID) == FALSE) {
        return NULL;
    }

    for (i = 0; i < npcCount; i++) {
        mapObj = MapObjMan_LocalMapObjByIndex(fieldSystem->mapObjMan, i);

        if (mapObj == NULL) {
            continue;
        }

        if (mode == VS_SEEKER_2V2_TRAINER_SEARCH_MODE_REMATCH_ANIM_CHECK) {
            if (MapObject_GetMoveCode(mapObj) == 0x31) {
                continue;
            }
        }

        eventType = MapObject_GetEventType(mapObj);

        switch (eventType) {
        case 0x1:
        case 0x2:
        case 0x4:
        case 0x5:
        case 0x6:
        case 0x7:
        case 0x8:
            secondTrainerEventID = MapObject_GetEventID(mapObj);
            secondTrainerID = Script_GetTrainerIDFromEventID(secondTrainerEventID);

            if ((eventID != secondTrainerEventID) && (trainerID == secondTrainerID)) {
                return mapObj;
            }
        }
    }

    return NULL;
}
