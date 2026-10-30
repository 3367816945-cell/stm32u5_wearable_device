/**
  ******************************************************************************
  * @file    networka_data_params.c
  * @author  AST Embedded Analytics Research Platform
  * @date    2026-07-14T17:03:47+0800
  * @brief   AI Tool Automatic Code Generator for Embedded NN computing
  ******************************************************************************
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */

#include "networka_data_params.h"


/**  Activations Section  ****************************************************/
ai_handle g_networka_activations_table[1 + 2] = {
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
  AI_HANDLE_PTR(NULL),
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
};




/**  Weights Section  ********************************************************/
AI_ALIGNED(32)
const ai_u64 s_networka_weights_array_u64[154] = {
  0x3fa50471be52f25aU, 0xbd2690873eaf523bU, 0xbd3a4340bdabdd19U, 0x3f03d8e2bfbafde0U,
  0xbf9ed167bead462aU, 0x3d17f1ef3de8fbddU, 0xbe17e5073d3fc026U, 0xbf6dec533fbfe3c8U,
  0xbed2194bbf3afadbU, 0xbe46ca00be8a1714U, 0xbc9193f9bd830d8dU, 0x3ebf0f6d3e1c0510U,
  0xbef163603eb92e6bU, 0xbdd5171e3ed995daU, 0xbe37efbcbe8d7eceU, 0x3d0e4324bf850574U,
  0xbf6e37a53cd36ea7U, 0x3d799fb7be686fdfU, 0xbe3a853e3dda2122U, 0xbfd1cb523feba6b3U,
  0xbdef585f3e220317U, 0xbebb9aa8be103b83U, 0x3eb056363ec2fb3cU, 0x3e464bc1bd153b7aU,
  0x3f3fb094bf474cb2U, 0x3cd5e4683d592ff3U, 0x3c5370ca3d2b6242U, 0xbe12b38ebf3c6e5cU,
  0xbf38b0a43eb74eecU, 0x3d5c4d303f67c2cfU, 0xbd8971d9be4d3960U, 0xbe025d77bf76eecaU,
  0x3f76d4aa3e1c401dU, 0xbf6d68793e216317U, 0xbde74755bf5f4f42U, 0xbf364bc83e586a6eU,
  0x3f8c3dbbbee93a53U, 0x3eb41e76bf21a75bU, 0x3e10d83d3f39bc6fU, 0x3d9a75e43f256bc3U,
  0x3f384a1fbf5a5e90U, 0xbbca4483bd29abb0U, 0xbc82529a3d8f15afU, 0xbdeb7270bf3cd7f0U,
  0x3fa9507fbf9d7accU, 0xbf9786c33f4af35bU, 0xbccd0fc2bf646fceU, 0xbea2bba8bf729e9aU,
  0x3dbc57943e02c177U, 0x3d0719a8be97a07cU, 0xbdbd75ce3d8f6407U, 0x3f9f57afbfa383ceU,
  0x40566129bed82118U, 0xbecf11f43e8bda7fU, 0xbd4eab4ebed7a562U, 0xbfad206ec0008a0cU,
  0x3f58c40abf52df8bU, 0xbf598fa0be4e3f4eU, 0x3d165b11bf03c473U, 0xbe95b86abeab2f76U,
  0xbe0d33423ebef11aU, 0xbefea06fbecf335eU, 0xbe87e3c6bebff637U, 0xbfbf7a7e3f91df4aU,
  0xbea78e9dbecba54cU, 0xbf011501bec076ceU, 0xbea9405cbecacf81U, 0xbe9ef108bf1c6893U,
  0x3d8e973bbd1a274fU, 0xbf0a3f74bf30c1bcU, 0xbe0eb1c1be4baa02U, 0xbe86d56ebf3a9178U,
  0xbf3b9edcbf0f3908U, 0x3e290351bf2635d8U, 0x3e412a8fbf055d56U, 0x3ca0ceb0bf071187U,
  0x3f0c05943cc6373dU, 0xbf3776c0bf29eca8U, 0xbf575405bf4cd87dU, 0xbf16f2bcbecbc591U,
  0xbe4292c6bf2c76d1U, 0xbeb6be6fbf92481bU, 0x3cd3b5c8bf031f5bU, 0xbed893d2bfcf52e4U,
  0x3f8812843f14f1d2U, 0x3e998507bfae1bd2U, 0xbe74e002bf738296U, 0x3d808a57bf29cedfU,
  0x3f398c303f0a317cU, 0xbf9eb9843ebb34aaU, 0xbefbc20c3e62be37U, 0xbf8f484abf7ba739U,
  0x3dadde153ef33472U, 0x3eda4906bf4bac1aU, 0x3f27add63eca0aeeU, 0x3f7545eebf2b469bU,
  0x3f948c33befb49c4U, 0xbf3ac9743f8cc0b1U, 0xbf08d048bf6565e9U, 0xbed49c6d3e084ab3U,
  0xbe45fc11bee84faaU, 0x3fb1f1d23de00b31U, 0x3f878fb23dcc2961U, 0x3d92a65d3fbf87b8U,
  0xbf6c14823e7e91adU, 0xbefc5fe73ec19555U, 0xbf122dc4402271c0U, 0xbf0b9d9cbef66a25U,
  0x3ef1c5e43f249756U, 0x3f115538be8a22bcU, 0x3f3e01913f509a48U, 0xbf0ca438bf0821c7U,
  0xbe9e96adbe1cf608U, 0x3ee6641d3eb0c232U, 0x3ec2730fbe5ce5daU, 0xbe809a833f4d5f02U,
  0xbea9693ebeadebdfU, 0x3f1dc7f13f2ed45dU, 0x3e91119abd5087ceU, 0xbe8395ea3fcce723U,
  0xbf01ab5abe3ca171U, 0x3f1574e63d80611aU, 0x3ea3ceacbee2f9c3U, 0x3eac93103ef4d0d9U,
  0xbe9125c7bed73813U, 0x3ea86dac3f2132b0U, 0x3da6932fbcfb1205U, 0xbedd3e9a3f9ffe15U,
  0x3f1e96af3ff11668U, 0xbfa2513b3f810740U, 0xbe89b2603e065e54U, 0xbf4d53aebf51e09bU,
  0xbef398ae3e1ff216U, 0xbe15e14abf2f5675U, 0xbd6b8ba7bf4df1ecU, 0x3f51b648be112455U,
  0x3f930ff53f58d24fU, 0x3bbd9d3bbdac869fU, 0xbcd6d0f3bf79471bU, 0x3ed3de1d3e6f2471U,
  0xbf8dc7b9bf567a0eU, 0x3ead2144bf86de2aU, 0x3eb307dbbf08cbd0U, 0xbf21ded23f087c36U,
  0x3ef66d183ee81144U, 0xbf07973fbdd4f6b4U, 0xbe7f9f0ebeae829eU, 0x3cdcd3b53e19e69aU,
  0x3e9b8d12bea6a793U, 0x3dd88c213f240944U, 0xbefd12ea3f4c9eabU, 0x3f3291f7bed63f7eU,
  0x3ee675f53e86a5bcU, 0xbe6e1c7dU,
};


ai_handle g_networka_weights_table[1 + 2] = {
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
  AI_HANDLE_PTR(s_networka_weights_array_u64),
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
};

