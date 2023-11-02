#include "ac_haniwa.h"

#include "m_play.h"
#include "m_bgm.h"
#include "m_rcp.h"
#include "m_name_table.h"
#include "m_house.h"
#include "m_font.h"
#include "m_msg.h"
#include "m_choice.h"
#include "m_demo.h"
#include "m_player.h"
#include "m_player_lib.h"
#include "m_clip.h"
#include "m_event.h"
#include "ac_intro_demo.h"
#include "ac_my_house.h"
#include "m_needlework_ovl.h"
#include "m_npc.h"
#include "m_submenu.h"
#include "m_field_info.h"
#include "m_common_data.h"

extern cKF_Skeleton_R_c cKF_bs_r_hnw;
extern cKF_Animation_R_c cKF_ba_r_hnw_move;
extern u8 hnw_tmem_txt[];
extern u8 hnw_face[];

// #include ac_haniwa_move_procs.c_inc

static void aHNW_actor_ct(ACTOR* actor, GAME* game);
static void aHNW_actor_dt(ACTOR* actor, GAME* game);
static void aHNW_actor_init(ACTOR* actor, GAME* game);

ACTOR_PROFILE Haniwa_Profile = {
  mAc_PROFILE_HANIWA,
  ACTOR_PART_BG,
  ACTOR_STATE_NONE,
  ACTOR_PROP_HANIWA0,
  ACTOR_OBJ_BANK_12,
  sizeof(HANIWA_ACTOR),

  &aHNW_actor_ct,
  &aHNW_actor_dt,
  &aHNW_actor_init,
  (mActor_proc)&none_proc1,
  NULL
};

static ClObjPipeData_c AcHaniwaCoInfoData = {
  { 57, 32, ClObj_TYPE_PIPE },
  { 1 },
  { 20, 30, 0, { 0, 0, 0 } }
};

static StatusData_c AcHaniwaStatusData = {
  0,
  20, 30, 0,
  254
};

/* TODO: ct, dt, & draw are in their own TU */
static void aHNW_actor_ct(ACTOR* actor, GAME* game) {
  HANIWA_ACTOR* haniwa = (HANIWA_ACTOR*)actor;
  ClObjPipe_c* pipe;
  cKF_SkeletonInfo_R_c* keyframe = &haniwa->keyframe;
  GAME_PLAY* play = (GAME_PLAY*)game;

  cKF_SkeletonInfo_R_ct(keyframe, &cKF_bs_r_hnw, NULL, haniwa->keyframe_work_area, haniwa->keyframe_morph_area);

  pipe = &haniwa->col_pipe;
  ClObjPipe_ct((GAME_PLAY*)game, pipe);
  ClObjPipe_set5((GAME_PLAY*)game, pipe, actor, &AcHaniwaCoInfoData);
  CollisionCheck_Status_set3(&haniwa->actor_class.status_data, &AcHaniwaStatusData);

  {
    Object_Bank_c* bank = &play->object_exchange.banks[actor->data_bank_id];
    haniwa->bank_ram_start = bank->ram_start;
  }

  haniwa->animation_state = 2;
  haniwa->house_idx = actor->npc_id - ACTOR_PROP_HANIWA0;
  actor->talk_distance = 43.0f;
}

static void aHNW_actor_dt(ACTOR* actor, GAME* game) {
  HANIWA_ACTOR* haniwa = (HANIWA_ACTOR*)actor;
  GAME_PLAY* play = (GAME_PLAY*)game;

  if (haniwa->playing_save_bgm) {
    mBGMPsComp_delete_ps_demo(0x41, 0x168);
  }

  cKF_SkeletonInfo_R_dt(&haniwa->keyframe);
  ClObjPipe_dt(play, &haniwa->col_pipe);
}


static Gfx hnw_tex_model[2];

static void aHNW_actor_draw(ACTOR* actor, GAME* game) {
  // TODO: this should be fixed when possible.
  // we need to add an additional TU for correct ordering
  // but PPCdis does not work well with that
  
  /*
  static Gfx hnw_tex_model[] = {
    gsDPLoadTLUT_Dolphin(15, 16, 1, hnw_face),
    gsSPEndDisplayList(),
  };
  */

  HANIWA_ACTOR* haniwa = (HANIWA_ACTOR*)actor;
  cKF_SkeletonInfo_R_c* keyframe = &haniwa->keyframe;
  GRAPH* g = game->graph;
  Mtx* m;
    
  m = GRAPH_ALLOC_TYPE(g, Mtx, keyframe->skeleton->num_shown_joints);

  if (m != NULL) {
    Gfx* gfx;
    int house_idx = haniwa->house_idx;
    _texture_z_light_fog_prim(g);

    OPEN_DISP(g);
    gfx = NOW_POLY_OPA_DISP;

    gSPSegment(gfx++, G_MWO_SEGMENT_B, hnw_tmem_txt);

    if (mPr_NullCheckPersonalID(&Save_Get(homes[house_idx]).ownerID) != TRUE &&
        Common_Get(player_no) == mHS_get_pl_no(house_idx)
    ) {
      gDPSetPrimColor(gfx++, 0, 128, 255, 255, 255, 255);
    }
    else {
      gDPSetPrimColor(gfx++, 0, 128, 255, 255, 255, 255);
    }

    gSPDisplayList(gfx++, hnw_tex_model);

    SET_POLY_OPA_DISP(gfx);
    CLOSE_DISP(g);

    cKF_Si3_draw_R_SV(game, keyframe, m, NULL, NULL, actor);
  }
}

static void aHNW_setupAction(ACTOR* actor, GAME* game, int action);

static int aHNW_set_save_permission() {
  u32 player_no = Common_Get(player_no);
  PersonalID_c* pid;
  mHm_hs_c* house;
  int res = FALSE;

  if (player_no < PLAYER_NUM) {
    int arrange_idx = mHS_get_arrange_idx(player_no);
    house = Save_GetPointer(homes[arrange_idx]);
    pid = &Save_Get(private[player_no]).player_ID;

    if (mPr_NullCheckPersonalID(pid) != TRUE &&
        mPr_CheckCmpPersonalID(pid, &house->ownerID) == TRUE
    ) {
      res = TRUE;
      house->flags.has_saved = TRUE;
    }
  }

  return res;
}

static void aHNW_search_player(ACTOR* actor) {
  chase_angle(&actor->shape_info.rotation.y, actor->player_angle_y, 0x0600);
}

static void aHNW_search_front(ACTOR* actor, int house_idx) {
  s16 target_angle[mHS_HOUSE_NUM] = { 8000, -8000, 8000, -8000 };

  chase_angle(&actor->shape_info.rotation.y, target_angle[house_idx], 0x0600);
}

static int aHNW_check_keep_item(ACTOR* actor) {
  HANIWA_ACTOR* haniwa = (HANIWA_ACTOR*)actor;
  Haniwa_Item_c* haniwa_item = Save_Get(homes[haniwa->house_idx]).haniwa.items;
  int i;
  int res = FALSE;

  for (i = 0; i < HANIWA_ITEM_HOLD_NUM; i++) {
    if (haniwa_item->item != EMPTY_NO) {
      res = TRUE;
      break;
    }
    
    haniwa_item++;
  }

  return res;
}

static void aHNW_set_proceeds_str(Haniwa_c* haniwa) {
  u8 str[7];

  mFont_UnintToString(str, 7, haniwa->bells, 6, TRUE, FALSE, TRUE);
  mMsg_Set_free_str(mMsg_Get_base_window_p(), 0, str, 7);
}

static int aHNW_check_handOver_proceeds(Haniwa_c* haniwa) {
  int handOver = FALSE;
  u32 money = Common_Get(now_private)->inventory.wallet + haniwa->bells;

  if (money < mPr_WALLET_MAX) {
    Common_Get(now_private)->inventory.wallet = money;
    handOver = TRUE;
  }
  else {
    u32 num_30k_bell_bags = (money - mPr_WALLET_MAX) / 30000 + 1;

    if (mPr_GetPossessionItemSumWithCond(Common_Get(now_private), EMPTY_NO, mPr_ITEM_COND_NORMAL) >= num_30k_bell_bags) {
      handOver = TRUE;

      for (num_30k_bell_bags; num_30k_bell_bags != 0; num_30k_bell_bags--) {
        mPr_SetFreePossessionItem(Common_Get(now_private), ITM_MONEY_30000, mPr_ITEM_COND_NORMAL);
        money -= 30000;
      }

      Common_Get(now_private)->inventory.wallet = money;
    }
    else {
      u8 str[5];

      mFont_UnintToString(str, 5, num_30k_bell_bags, 5, TRUE, FALSE, TRUE);
      mMsg_Set_free_str(mMsg_Get_base_window_p(), 1, str, 5);
    }
  }

  return handOver;
}

static int aHNW_check_house_door(ACTOR* actor, GAME* game) {
  HANIWA_ACTOR* haniwa = (HANIWA_ACTOR*)actor;
  int res = FALSE;
  GAME_PLAY* play = (GAME_PLAY*)game;

  if (Common_Get(reset_flag) == TRUE) {
    ACTOR* talk_actor = mDemo_Get_talk_actor();

    if (talk_actor == NULL && chkTrigger(BUTTON_A)) {
      PLAYER_ACTOR* player = get_player_actor_withoutCheck(play);

      if (mDemo_Check_DiffAngle_forTalk((actor->player_angle_y - player->actor_class.shape_info.rotation.y) - -0x8000) == TRUE) {
        Common_Set(reset_type, 4);
      }
    }

    res = TRUE;
  }
  else {
    mDemo_Clip_c* demo_clip = Common_Get(clip).demo_clip;

    if (demo_clip != NULL) {
      INTRO_DEMO_ACTOR* demo_class = (INTRO_DEMO_ACTOR*)demo_clip->class;
      if (demo_class != NULL && demo_clip->type == mDemo_CLIP_TYPE_INTRO_DEMO &&
          mEv_CheckFirstIntro() && demo_class->player_intro_demo_state != 0) {
        res = TRUE;
      }
    }
  }

  return res;
}

static void aHNW_wait(ACTOR* actor, GAME* game) {
  if (actor->player_distance_xz < 80.0f) {
    aHNW_setupAction(actor, game, aHNW_ACTION_DANCE);
  }
}

static int aHNW_decide_msg_idx_dance(ACTOR* actor) {
  HANIWA_ACTOR* haniwa = (HANIWA_ACTOR*)actor;
  Haniwa_c* house_haniwa;
  int house_idx = haniwa->house_idx;
  int player_is_owner = Common_Get(player_no) == mHS_get_pl_no(haniwa->house_idx);
  int res;


  if (mPr_NullCheckPersonalID(&Save_Get(homes[house_idx]).ownerID) == TRUE) {
    res = aHNW_MSG_NO_OWNER; /* no one owns this house */
  }
  else if (player_is_owner == TRUE) {
    Haniwa_c* house_haniwa = &Save_Get(homes[house_idx]).haniwa;
    int house_arrange_idx = mHS_get_arrange_idx(Common_Get(player_no));
    mHm_hs_c* house = Save_GetPointer(homes[house_arrange_idx]);
    if (house->flags.has_saved == FALSE &&
        mEv_CheckFirstJob() == TRUE && mNpc_GetFriendAnimalNum(&Common_Get(now_private)->player_ID) == 0
    ) {
      res = aHNW_MSG_NEED_FRIEND; /* player owns this house, but is in intro and has not spoken to any villagers */
    }
    else if (house_haniwa->bells != 0) {
      res = aHNW_MSG_PROCEEDS;
    }
    else {
      res = aHNW_MSG_NORMAL;
    }

    /* player owns house but gyroid has bells from other players */
  }
  else {
    res = aHNW_MSG_OTHER_OWNER; /* another player owns house */
  }

  return res;
}


static void aHNW_set_talk_info_dance(ACTOR* actor) {
  static int msg_no[aHNW_MSG_NUM] = {
    0x0934, /* aHNW_MSG_NO_OWNER */
    0x0935, /* aHNW_MSG_PROCEEDS */
    0x0925, /* aHNW_MSG_NORMAL */
    0x0928, /* aHNW_MSG_OTHER_OWNER */
    0x092E  /* aHNW_MSG_NEED_FRIEND */
  };

  HANIWA_ACTOR* haniwa = (HANIWA_ACTOR*)actor;
  int house_idx = haniwa->house_idx;
  Haniwa_c* house_haniwa = &Save_Get(homes[house_idx]).haniwa;
  int msg_idx = aHNW_decide_msg_idx_dance(actor);

  switch (msg_idx) {
    case aHNW_MSG_PROCEEDS:
    {
      aHNW_set_proceeds_str(house_haniwa);
      break;
    }

    case aHNW_MSG_OTHER_OWNER:
    {
      mMsg_Set_mail_str(mMsg_Get_base_window_p(), 0, house_haniwa->message, HANIWA_MESSAGE_LEN);
      break;
    }
  }

  mDemo_Set_msg_num(msg_no[msg_idx]);
}

static void aHNW_dance(ACTOR* actor, GAME* game) {
  static int next_act_idx[aHNW_MSG_NUM] = {
    aHNW_ACTION_TALK_END_WAIT, /* aHNW_MSG_NO_OWNER */
    aHNW_ACTION_CHECK_PROCEEDS, /* aHNW_MSG_PROCEEDS */
    aHNW_ACTION_TALK_WITH_MASTER, /* aHNW_MSG_NORMAL */
    aHNW_ACTION_TALK_WITH_GUEST, /* aHNW_MSG_OTHER_OWNER */
    aHNW_ACTION_TALK_END_WAIT  /* aHNW_MSG_NEED_FRIEND */
  };

  HANIWA_ACTOR* haniwa = (HANIWA_ACTOR*)actor;

  if (actor->player_distance_xz > 90.0f) {
    aHNW_setupAction(actor, game, aHNW_ACTION_WAIT);
  }
  else {
    if (mDemo_Check(mDemo_TYPE_TALK, (ACTOR*)haniwa) == TRUE && mDemo_Check_ListenAble() == FALSE) {
      int msg_idx = aHNW_decide_msg_idx_dance(actor);
      mDemo_Set_ListenAble();
      aHNW_setupAction(actor, game, next_act_idx[msg_idx]);
    }
    else if (aHNW_check_house_door((ACTOR*)haniwa, game) == FALSE) {
      mDemo_Request(mDemo_TYPE_TALK, (ACTOR*)haniwa, &aHNW_set_talk_info_dance);
    }
  }
}

static void aHNW_check_proceeds(ACTOR* actor, GAME* game) {
  static int msg_no[aHNW_HANDOVER_NUM] = {
    0x0936, /* aHNW_HANDOVER_YES */
    0x0937  /* aHNW_HANDOVER_NO */
  };

  static int next_act_idx[aHNW_HANDOVER_NUM] = {
    aHNW_ACTION_TALK_WITH_MASTER, /* aHNW_HANDOVER_YES */
    aHNW_ACTION_TALK_END_WAIT  /* aHNW_HANDOVER_NO */
  };

  HANIWA_ACTOR* haniwa = (HANIWA_ACTOR*)actor;

  if (mDemo_Get_OrderValue(mDemo_ORDER_NPC0, 9)) {
    int house_idx = haniwa->house_idx;
    Haniwa_c* house_haniwa = &Save_Get(homes[house_idx]).haniwa;
    int handover_state;

    if (aHNW_check_handOver_proceeds(house_haniwa) == TRUE) {
      house_haniwa->bells = 0;
      handover_state = aHNW_HANDOVER_YES;
    }
    else {
      handover_state = aHNW_HANDOVER_NO;
    }

    mMsg_Set_continue_msg_num(mMsg_Get_base_window_p(), msg_no[handover_state]);
    aHNW_setupAction(actor, game, next_act_idx[handover_state]);
    mDemo_Set_OrderValue(mDemo_ORDER_NPC0, 9, 0);
  }
}

static void aHNW_talk_with_master(ACTOR* actor, GAME* game) {
  HANIWA_ACTOR* haniwa = (HANIWA_ACTOR*)actor;
  mMsg_Window_c* msg_win = mMsg_Get_base_window_p();

  if (mDemo_Get_OrderValue(mDemo_ORDER_NPC0, 9) && mMsg_Check_MainNormalContinue(msg_win) == TRUE) {
    int action = -1;

    switch (mChoice_Get_ChoseNum(mChoice_Get_base_window_p())) {
      case mChoice_CHOICE0:
      {
        action = aHNW_ACTION_SAVE_CHECK;
        break;
      }

      case mChoice_CHOICE1:
      {
        action = aHNW_ACTION_MENU_OPEN_WAIT;
        haniwa->submenu_type = mSM_OVL_INVENTORY;
        break;
      }

      case mChoice_CHOICE2:
      {
        action = aHNW_ACTION_TALK_WITH_MASTER2;
        break;
      }

      case mChoice_CHOICE3:
      {
        action = aHNW_ACTION_TALK_END_WAIT;
        break;
      }
    }

    if (action != -1) {
      mDemo_Set_OrderValue(mDemo_ORDER_NPC0, 9, 0);
      aHNW_setupAction(actor, game, action);
    }
  }
}

static void aHNW_talk_with_master2(ACTOR* actor, GAME* game) {
  HANIWA_ACTOR* haniwa = (HANIWA_ACTOR*)actor;
  mMsg_Window_c* msg_win = mMsg_Get_base_window_p();

  if (mDemo_Get_OrderValue(mDemo_ORDER_NPC0, 9) && mMsg_Check_MainNormalContinue(msg_win) == TRUE) {
    int action = -1;

    switch (mChoice_Get_ChoseNum(mChoice_Get_base_window_p())) {
      case mChoice_CHOICE0:
      {
        action = aHNW_ACTION_ROOF_CHECK;
        break;
      }

      case mChoice_CHOICE1:
      {
        action = aHNW_ACTION_MENU_OPEN_WAIT;
        haniwa->submenu_type = mSM_OVL_HBOARD;
        break;
      }

      case mChoice_CHOICE2:
      {
        action = aHNW_ACTION_TALK_WITH_MASTER;
        break;
      }

      case mChoice_CHOICE3:
      {
        action = aHNW_ACTION_TALK_END_WAIT;
        break;
      }
    }

    if (action != -1) {
      mDemo_Set_OrderValue(mDemo_ORDER_NPC0, 9, 0);
      aHNW_setupAction(actor, game, action);
    }
  }
}

static void aHNW_talk_end_wait(ACTOR* actor, GAME* game) {
  if (mDemo_Check(mDemo_TYPE_TALK, actor) == FALSE) {
    aHNW_setupAction(actor, game, aHNW_ACTION_DANCE);
  }
}

static void aHNW_menu_open_wait(ACTOR* actor, GAME* game) {
  HANIWA_ACTOR* haniwa = (HANIWA_ACTOR*)actor;
  GAME_PLAY* play = (GAME_PLAY*)game;
  mMsg_Window_c* msg_win = mMsg_Get_base_window_p();

  if (mMsg_Check_main_wait(msg_win) == TRUE) {
    int submenu_type = haniwa->submenu_type;
    Submenu* submenu = &play->submenu;
    int arg1 = haniwa->house_idx;

    switch (submenu_type) {
      case mSM_OVL_HBOARD:
      {
        mSM_open_submenu(submenu, mSM_OVL_HBOARD, 0, arg1);
        break;
      }

      case mSM_OVL_INVENTORY:
      {
        mSM_open_submenu(submenu, mSM_OVL_INVENTORY, 2, arg1);
        break;
      }

      case mSM_OVL_NEEDLEWORK:
      {
        mSM_open_submenu(submenu, mSM_OVL_NEEDLEWORK, 0, arg1);
        break;
      }
    }

    mMsg_ChangeMsgData(msg_win, 0x0927);
    mMsg_Set_ForceNext(msg_win);
    aHNW_setupAction(actor, game, aHNW_ACTION_MENU_END_WAIT);
  }
}

static void aHNW_menu_end_wait(ACTOR* actor, GAME* game) {
  HANIWA_ACTOR* haniwa = (HANIWA_ACTOR*)actor;
  GAME_PLAY* play = (GAME_PLAY*)game;
  Submenu* submenu = &play->submenu;
  Submenu_Item_c* item;

  if (submenu->flag == FALSE) {
    if (mMsg_Check_not_series_main_wait(mMsg_Get_base_window_p()) == TRUE) {
      aHNW_setupAction(actor, game, aHNW_ACTION_TALK_WITH_MASTER);
      switch (haniwa->submenu_type) {
        case mSM_OVL_NEEDLEWORK:
        {
          Submenu_Item_c* item = submenu->item_p;
          if (item->item == RSV_NO) {
            Save_Get(homes[haniwa->house_idx]).door_original = mNW_get_image_no(submenu, item->slot_no);
            sAdo_SysTrgStart(0x461);
          }

          break;
        }
      }

      haniwa->submenu_type = mSM_OVL_NONE;
    }
  }
}

static void aHNW_talk_with_guest(ACTOR* actor, GAME* game) {
  HANIWA_ACTOR* haniwa = (HANIWA_ACTOR*)actor;

  if (mDemo_Get_OrderValue(mDemo_ORDER_NPC0, 0)) {
    mMsg_Window_c* msg_win = mMsg_Get_base_window_p();
    if (mMsg_Check_MainNormalContinue(msg_win) == TRUE) {
      int action = -1;

      switch (mChoice_Get_ChoseNum(mChoice_Get_base_window_p())) {
        case mChoice_CHOICE0:
        {
          if (aHNW_check_keep_item(actor) == FALSE) {
            mMsg_Set_continue_msg_num(msg_win, 0x092C);
            action = aHNW_ACTION_TALK_END_WAIT;
          }
          else {
            action = aHNW_ACTION_MENU_OPEN_WAIT_FOR_GUEST;
          }
          break;
        }

        case mChoice_CHOICE1:
        {
          mMsg_Set_continue_msg_num(msg_win, 0x092A);
          action = aHNW_ACTION_TALK_END_WAIT;
          break;
        }
      }

      if (action != -1) {
        mDemo_Set_OrderValue(mDemo_ORDER_NPC0, 0, 0);
        aHNW_setupAction(actor, game, action);
      }
    }
  }
}

static void aHNW_menu_open_wait_for_guest(ACTOR* actor, GAME* game) {
  HANIWA_ACTOR* haniwa = (HANIWA_ACTOR*)actor;
  GAME_PLAY* play = (GAME_PLAY*)game;
  mMsg_Window_c* msg_win = mMsg_Get_base_window_p();

  if (mMsg_Check_main_wait(msg_win) == TRUE) {
    mSM_open_submenu(&play->submenu, mSM_OVL_INVENTORY, 3, haniwa->house_idx);
    mMsg_ChangeMsgData(msg_win, 0x092B);
    aHNW_setupAction(actor, game, aHNW_ACTION_MENU_END_WAIT_FOR_GUEST);
  }
}

static void aHNW_menu_end_wait_for_guest(ACTOR* actor, GAME* game) {
  HANIWA_ACTOR* haniwa = (HANIWA_ACTOR*)actor;
  GAME_PLAY* play = (GAME_PLAY*)game;

  if (play->submenu.flag == FALSE && mMsg_Check_not_series_main_wait(mMsg_Get_base_window_p()) == TRUE) {
    aHNW_setupAction(actor, game, aHNW_ACTION_TALK_END_WAIT);
  }
}

static void aHNW_save_check(ACTOR* actor, GAME* game) {
  HANIWA_ACTOR* haniwa = (HANIWA_ACTOR*)actor;
  mMsg_Window_c* msg_win = mMsg_Get_base_window_p();
  int order_value = mDemo_Get_OrderValue(mDemo_ORDER_NPC0, 9);
  int action = -1;

  if (order_value && mMsg_Check_MainNormalContinue(msg_win) == TRUE) {
    switch (mChoice_Get_ChoseNum(mChoice_Get_base_window_p())) {
      case mChoice_CHOICE0:
      {
        action = aHNW_ACTION_SAVE_END_WAIT;
        mBGMPsComp_scene_mode(13);
        mDemo_Set_talk_return_demo_wait(1);
        mPlib_Set_able_force_speak_label(actor);
        break;
      }

      case mChoice_CHOICE1:
      {
        action = aHNW_ACTION_TALK_WITH_MASTER;
        break;
      }
    }

    if (action != -1) {
      mDemo_Set_OrderValue(mDemo_ORDER_NPC0, 9, 0);
      aHNW_setupAction(actor, game, action);
    }
  }
}

static void aHNW_roof_check(ACTOR* actor, GAME* game) {
  HANIWA_ACTOR* haniwa = (HANIWA_ACTOR*)actor;
  mMsg_Window_c* msg_win = mMsg_Get_base_window_p();
  int order_value = mDemo_Get_OrderValue(mDemo_ORDER_NPC0, 9);
  int action = -1;

  if (order_value && mMsg_Check_MainNormalContinue(msg_win) == TRUE) {
    switch (mChoice_Get_ChoseNum(mChoice_Get_base_window_p())) {
      case mChoice_CHOICE0:
      {
        action = aHNW_ACTION_MENU_OPEN_WAIT;
        haniwa->submenu_type = mSM_OVL_NEEDLEWORK;
        break;
      }

      case mChoice_CHOICE1:
      {
        Save_Get(homes[haniwa->house_idx]).door_original = 0xFF;
        sAdo_SysTrgStart(0x461);
        action = aHNW_ACTION_TALK_WITH_MASTER;
        break;
      }

      case mChoice_CHOICE2:
      {
        action = aHNW_ACTION_TALK_WITH_MASTER;
        break;
      }
    }

    if (action != -1) {
      mDemo_Set_OrderValue(mDemo_ORDER_NPC0, 9, 0);
      aHNW_setupAction(actor, game, action);
    }
  }
}

static void aHNW_save_end_wait(ACTOR* actor, GAME* game) {
  HANIWA_ACTOR* haniwa = (HANIWA_ACTOR*)actor;
  
  if (haniwa->playing_save_bgm == FALSE && mMsg_Check_MainDisappear(mMsg_Get_base_window_p())) {
    haniwa->playing_save_bgm = TRUE;
    mBGMPsComp_make_ps_demo(0x41, 0x168);
  }

  if (mDemo_Check(mDemo_TYPE_TALK, actor) == FALSE) {
    aHNW_setupAction(actor, game, aHNW_ACTION_PL_APPROACH_DOOR);
    aHNW_set_save_permission();
  }
}


static void aHNW_pl_approach_door(ACTOR* actor, GAME* game) {
  static f32 chk_posX[mHS_HOUSE_NUM] = { 2095.0f, 2385.0f, 2095.0f, 2385.0f };
  static f32 chk_val[mHS_HOUSE_NUM] = { 1.0f, -1.0f, 1.0f, -1.0f };
  static xyz_t goal_pos[mHS_HOUSE_NUM][2] = {
    {
      { 2098.0f, 0.0f, 1540.0f },
      { 2110.0f, 0.0f, 1474.0f }
    },
    {
      { 2382.0f, 0.0f, 1540.0f },
      { 2369.0f, 0.0f, 1474.0f }
    },
    {
      { 2098.0f, 0.0f, 1820.0f },
      { 2110.0f, 0.0f, 1755.0f }
    },
    {
      { 2382.0f, 0.0f, 1820.0f },
      { 2369.0f, 0.0f, 1755.0f }
    }
  };

  HANIWA_ACTOR* haniwa = (HANIWA_ACTOR*)actor;
  GAME_PLAY* play = (GAME_PLAY*)game;
  PLAYER_ACTOR* player = get_player_actor_withoutCheck((GAME_PLAY*)game);

  haniwa->door_approach_frame++;

  if (player != NULL) {
    int house_idx = haniwa->house_idx;
    int stage = ((chk_posX[house_idx] - player->actor_class.world.position.x) * chk_val[house_idx]) <= 0.0f;
    xyz_t* goal = goal_pos[house_idx] + stage;

    if (haniwa->player_approach_door_stage != stage && mPlib_request_main_demo_walk_type1(game, goal->x, goal->z, 3.0f, FALSE)) {
      haniwa->player_approach_door_stage = stage;
    }

    mPlib_Set_goal_player_demo_walk(goal->x, goal->z, 3.0f);

    if (haniwa->door_approach_frame > 160) {
      MY_HOUSE_ACTOR* house_actor = (MY_HOUSE_ACTOR*)Actor_info_fgName_search(&play->actor_info, HOUSE0 + haniwa->house_idx, ACTOR_PART_ITEM);

      if (house_actor != NULL) {
        house_actor->actor_class.world.angle.z = 1;
        aHNW_setupAction(actor, game, aHNW_ACTION_WAIT);
      }
    }
    else if (stage == 1 && search_position_distanceXZ(goal, &player->actor_class.world.position) < 3.0f) {
      aHNW_setupAction(actor, game, aHNW_ACTION_DOOR_OPEN_WAIT);
    }
  }
}
#pragma pool_data reset

static void aHNW_door_open_wait(ACTOR* actor, GAME* game) {
  HANIWA_ACTOR* haniwa = (HANIWA_ACTOR*)actor;
  GAME_PLAY* play = (GAME_PLAY*)game;
  MY_HOUSE_ACTOR* house_actor = (MY_HOUSE_ACTOR*)Actor_info_fgName_search(&play->actor_info, HOUSE0 + haniwa->house_idx, ACTOR_PART_ITEM);

  if (house_actor != NULL) {
    house_actor->door_rotation_speed_idx = 6; // TODO: this is probably an enum
    if (house_actor != get_player_actor_withoutCheck((GAME_PLAY*)gamePT)->get_door_label_proc(gamePT)) {
      aHNW_setupAction(actor, game, aHNW_ACTION_DOOR_OPEN_TIMER);
      haniwa->door_approach_frame = 0;
    }
    else {
      aHNW_setupAction(actor, game, aHNW_ACTION_WAIT);
    }
  }
}

static void aHNW_door_open_timer(ACTOR* actor, GAME* game) {
  HANIWA_ACTOR* haniwa = (HANIWA_ACTOR*)actor;
  GAME_PLAY* play = (GAME_PLAY*)game;
  MY_HOUSE_ACTOR* house_actor = (MY_HOUSE_ACTOR*)Actor_info_fgName_search(&play->actor_info, HOUSE0 + haniwa->house_idx, ACTOR_PART_ITEM);

  if (house_actor != NULL) {
    if (house_actor == get_player_actor_withoutCheck((GAME_PLAY*)gamePT)->get_door_label_proc(gamePT)) {
      aHNW_setupAction(actor, game, aHNW_ACTION_WAIT);
    }
    else {
      haniwa->door_approach_frame++;
      if (haniwa->door_approach_frame > 80) {
        house_actor->actor_class.world.angle.z = 1;
        aHNW_setupAction(actor, game, aHNW_ACTION_WAIT);
      }
    }
  }
}

static void aHNW_menu_open_wait_init(ACTOR* actor, GAME* game) {
  mMsg_request_main_disappear_wait_type1(mMsg_Get_base_window_p());
}

static void aHNW_menu_end_wait_init(ACTOR* actor, GAME* game) {
  mMsg_request_main_appear_wait_type1(mMsg_Get_base_window_p());
}

static void aHNW_talk_with_guest_init(ACTOR* actor, GAME* game) {
  HANIWA_ACTOR* haniwa = (HANIWA_ACTOR*)actor;
  
  mMsg_Set_free_str(mMsg_Get_base_window_p(), 2, Save_Get(homes[haniwa->house_idx]).ownerID.player_name, PLAYER_NAME_LEN);
}

static void aHNW_pl_approach_door_init(ACTOR* actor, GAME* game) {
  HANIWA_ACTOR* haniwa = (HANIWA_ACTOR*)actor;

  game->pad_initialized = FALSE;
  haniwa->player_approach_door_stage = -1;
  haniwa->door_approach_frame = 0;
}

typedef void (*HANIWA_PROC)(ACTOR*, GAME*);

static void aHNW_init_proc(ACTOR* actor, GAME* game, int action) {
  static HANIWA_PROC init_proc[aHNW_ACTION_NUM] = {
    (HANIWA_PROC)&none_proc1,
    (HANIWA_PROC)&none_proc1,
    (HANIWA_PROC)&none_proc1,
    (HANIWA_PROC)&none_proc1,
    (HANIWA_PROC)&none_proc1,
    (HANIWA_PROC)&none_proc1,
    &aHNW_menu_open_wait_init,
    &aHNW_menu_end_wait_init,
    &aHNW_talk_with_guest_init,
    &aHNW_menu_open_wait_init,
    &aHNW_menu_end_wait_init,
    (HANIWA_PROC)&none_proc1,
    (HANIWA_PROC)&none_proc1,
    (HANIWA_PROC)&none_proc1,
    &aHNW_pl_approach_door_init,
    (HANIWA_PROC)&none_proc1,
    (HANIWA_PROC)&none_proc1
  };

  (*init_proc[action])(actor, game);
}

static void aHNW_setupAction(ACTOR* actor, GAME* game, int action) {
  static HANIWA_PROC process[aHNW_ACTION_NUM] = {
    &aHNW_wait,
    &aHNW_dance,
    &aHNW_check_proceeds,
    &aHNW_talk_with_master,
    &aHNW_talk_with_master2,
    &aHNW_talk_end_wait,
    &aHNW_menu_open_wait,
    &aHNW_menu_end_wait,
    &aHNW_talk_with_guest,
    &aHNW_menu_open_wait_for_guest,
    &aHNW_menu_end_wait_for_guest,
    &aHNW_roof_check,
    &aHNW_save_check,
    &aHNW_save_end_wait,
    &aHNW_pl_approach_door,
    &aHNW_door_open_wait,
    &aHNW_door_open_timer
  };

  HANIWA_ACTOR* haniwa = (HANIWA_ACTOR*)actor;

  int house_idx = haniwa->house_idx;
  int animation_state = haniwa->animation_state;
  int no_owner = mPr_NullCheckPersonalID(&Save_Get(homes[house_idx]).ownerID);
  int owner_is_player = mHS_get_pl_no(house_idx) == Common_Get(player_no);

  haniwa->action = action;
  haniwa->action_proc = process[action];

  if (action >= aHNW_ACTION_CHECK_PROCEEDS) {
    haniwa->anim_frame_speed = 0.3f;
  }
  else {
    if (no_owner) {
      if (haniwa->anim_frame_speed != 0.0f) {
        haniwa->anim_frame_speed = 0.075f;
      }
    }
    else {
      if (owner_is_player) {
        if (action == aHNW_ACTION_WAIT || action == aHNW_ACTION_DOOR_OPEN_TIMER) {
          haniwa->anim_frame_speed = 0.3f;
        }
        else {
          haniwa->anim_frame_speed = 0.45f;
        }
      }
      else {
        haniwa->anim_frame_speed = 0.1f;
      }
    }
  }

  if (animation_state == 2) {
    cKF_SkeletonInfo_R_init(
      &haniwa->keyframe, haniwa->keyframe.skeleton, &cKF_ba_r_hnw_move,
      1.0f, 9.0f, 1.0f, haniwa->anim_frame_speed, 0.0f, 
      cKF_FRAMECONTROL_REPEAT,
      NULL
    );
    haniwa->saved_current_frame = haniwa->keyframe.frame_control.current_frame;
  }

  haniwa->animation_state = 0;
  aHNW_init_proc(actor, game, action);
  if (no_owner && action < aHNW_ACTION_CHECK_PROCEEDS) {
    haniwa->keyframe.frame_control.mode = cKF_FRAMECONTROL_STOP;
  }
  else {
    haniwa->keyframe.frame_control.mode = cKF_FRAMECONTROL_REPEAT;
  }
}

static void aHNW_common_process(ACTOR* actor, GAME* game) {
  HANIWA_ACTOR* haniwa = (HANIWA_ACTOR*)actor;
  cKF_SkeletonInfo_R_c* keyframe = &haniwa->keyframe;
  int house_idx = haniwa->house_idx;
  int no_owner = mPr_NullCheckPersonalID(&Save_Get(homes[house_idx]).ownerID);
  f32 target;

  if (no_owner == FALSE && keyframe->frame_control.mode == cKF_FRAMECONTROL_STOP) {
    aHNW_setupAction((ACTOR*)haniwa, game, haniwa->action);
    keyframe->frame_control.mode = cKF_FRAMECONTROL_REPEAT;
  }
  else if (no_owner && haniwa->action < 2 && keyframe->frame_control.speed <= 0.1f) {
    keyframe->frame_control.mode = cKF_FRAMECONTROL_STOP;
  }
  else {
    keyframe->frame_control.mode = cKF_FRAMECONTROL_REPEAT;
  }

  if (no_owner == FALSE || haniwa->action >= aHNW_ACTION_CHECK_PROCEEDS) {
    aHNW_search_player((ACTOR*)haniwa);
  }
  else {
    aHNW_search_front((ACTOR*)haniwa, house_idx);
  }

  target = haniwa->anim_frame_speed;

  {
    if (target > keyframe->frame_control.speed) {
      chase_f(&keyframe->frame_control.speed, target, 0.05f);
    }
    else {
      chase_f(&keyframe->frame_control.speed, target, 0.015f);
    }
  }
}

static void aHNW_actor_move(ACTOR* actor, GAME* game) {
  HANIWA_ACTOR* haniwa = (HANIWA_ACTOR*)actor;
  GAME_PLAY* play = (GAME_PLAY*)game;
  cKF_SkeletonInfo_R_c* keyframe = &haniwa->keyframe;

  haniwa->keyframe_state = cKF_SkeletonInfo_R_play(keyframe);
  (*haniwa->action_proc)((ACTOR*)haniwa, game);
  aHNW_common_process(actor, game);
  CollisionCheck_Uty_ActorWorldPosSetPipeC(actor, &haniwa->col_pipe);
  CollisionCheck_setOC(play, &play->collision_check, &haniwa->col_pipe.collision_obj);
  Actor_world_to_eye(actor, 50.0f);
}

static void aHNW_actor_init(ACTOR* actor, GAME* game) {
  HANIWA_ACTOR* haniwa = (HANIWA_ACTOR*)actor;
  int house_idx = haniwa->house_idx;

  mFI_SetFG_common((mActor_name_t)(house_idx + DUMMY_HANIWA0), actor->world.position, FALSE);
  actor->mv_proc = &aHNW_actor_move;
  actor->dw_proc = &aHNW_actor_draw;
  aHNW_setupAction((ACTOR*)haniwa, game, aHNW_ACTION_WAIT); // weird that we have to re-cast to ACTOR so fequently for matches
  haniwa->keyframe.morph_counter = 0.0f;
  aHNW_actor_move(actor, game);
}

/* TODO: hack */
static Gfx hnw_tex_model[] = {
  gsDPLoadTLUT_Dolphin(15, 16, 1, hnw_face),
  gsSPEndDisplayList(),
};
