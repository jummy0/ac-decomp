#ifndef AUDIO_H
#define AUDIO_H

#include "types.h"
#include "audio_defs.h"
#include "m_lib.h"

#ifdef __cplusplus
extern "C" {
#endif

/* sizeof(TempoBeat_c) == 2 */
typedef struct audio_tempo_beat_s {
  /* 0x00 */ u8 tempo;
  /* 0x01 */ s8 beat;
} TempoBeat_c;

extern void sAdo_Calc_MicPosition_forTrig(f32* fcalc, u16* scalc, xyz_t* pos);
extern void sAdo_Calc_MicPosition_forLevel(f32* fcalc, u16* scalc, xyz_t* pos);

extern void sAdo_GameFrame();
extern void sAdo_BgmStart(s8 id);
extern void sAdo_BgmStop(s16 id);
extern void sAdo_SysTrgStart(u16 id);

extern void sAdo_PlyWalkSe(int walk, xyz_t* pos);
extern void sAdo_PlyWalkSeRoom(int walk, xyz_t* pos);
extern void sAdo_NpcWalkSe(int walk, xyz_t* pos);
extern void sAdo_NpcWalkSeRoom(int walk, xyz_t* pos);

extern void sAdo_PlayerStatusLevel(f32 speed, int p);

/* Not sure about the last param name */
extern void sAdo_VoiceSe(u8 num, u8 num2, u8 num3, s16 character_idx, u8 scale, u8 mode);

extern void sAdo_Inst(u16 num, u8* p);
extern void sAdo_MessageStatus(u8 status);
extern void sAdo_MessageSpeed(u8 speed);
extern u8 sAdo_MessageSpeedGet();

extern void sAdo_SysLevStop(u8 id);
extern void sAdo_SysLevStart(u8 id);

extern void sAdo_OngenPos(u8 p1, u8 p2, xyz_t* pos);
extern void sAdo_OngenTrgStart(u16 id, xyz_t* pos);

extern void sAdo_SetOutMode(u8 mode);
extern void sAdo_SetVoiceMode(u8 mode);

extern void sAdo_FloorTrgStart(u16 id, xyz_t* pos);

extern void sAdo_RhythmStart(void* p, u8 b1, u8 b2);
extern void sAdo_RhythmStop(void* p);
extern void sAdo_RhythmAllStop();

extern void sAdo_FurnitureInst(int track_id, s8 cb, s32 cl, xyz_t* pos);
extern void sAdo_Pause(u8 mode);

extern f32 sAdo_GetRhythmAnimCounter(void* p);
extern f32 sAdo_GetRhythmDelay(void* p);
extern f32 sAdo_GetRhythmInfo(void* p);
extern void sAdo_SetRhythmInfo(f32 p);

extern int sAdo_InstCountGet();

extern void sAdo_RhythmPos(void* p, int unk, xyz_t* pos);

extern void sAdo_SpecChange(int spec);
extern void sAdo_MDPlayerPos(xyz_t* pos, u16 s1, u16 s2, int p);

extern void sAdo_BGMVolume(f32 f, u16 volume);
extern void sAdo_BgmCrossfadeStart(u8 b);

extern void sAdo_SubGameStart();
extern void sAdo_SubGameEnd();
extern int sAdo_SubGameOK();

extern void sAdo_Tenki(u8 mode);

extern void sAdos_KishaStatusTrg(u8 state);
extern void sAdos_KishaStatusLevel(f32 speed, u32 ongenNum1, u16 angle1, f32 distance1, u32 ongenNum2, u16 angle2, f32 distance2);

extern void sAdos_GetRadioCounter(void* p);
extern void sAdos_GetKappaCounter(void* p);
extern void sAdos_GetStaffRollInfo(void* p);

extern void sAdos_TTKK_ARM(u8 mode);

extern void sAdo_OngenTrgStartSpeed(f32 speed, u16 s1, xyz_t* pos);

extern void sAdo_TrgSeEcho(u8 b);
extern void sAdo_LevSeEcho(u8 b);

extern u8 sAdo_BgmFadeoutCheck();
extern u8 sAdo_SeFadeoutCheck();

extern void sAdo_RoomType(u8 type);
extern void sAdo_Museum(u8 type);

extern void sAdo_SoftReset();

extern u16 sAdo_Get_WalkLabel(int type);
extern int sAdo_Get_KokeruLabel(int type);

extern void sAdo_SceneMode(u8 mode);
extern void sAdo_RoomIncectPos(int f, u16 u, xyz_t* pos);

extern void sAdo_FurnitureInstPos(int f, xyz_t* pos);
extern void sAdo_Set_ongenpos_refuse_fg(int state);

extern int sAdo_GameframeEnd_Check();
extern int sAdo_GetSoundFrameCounter();

extern int sAdo_CheckOnPlussBridge(xyz_t* pos, u32 atr);
extern void sAdo_RegistPlussBridge(xyz_t* pos);
extern void sAdo_DeletePlussBridge();

extern void sAdo_kazagurumaLevel(f32 level);

#ifdef __cplusplus
}
#endif

#endif
