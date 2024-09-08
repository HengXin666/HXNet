#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-09-07 21:21:59
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	  https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _HX_UN_REFLECT_JSON_H_
#define _HX_UN_REFLECT_JSON_H_

#undef _REFLECT_PP_FOREACH_1
#undef _REFLECT_PP_FOREACH_2
#undef _REFLECT_PP_FOREACH_3
#undef _REFLECT_PP_FOREACH_4
#undef _REFLECT_PP_FOREACH_5
#undef _REFLECT_PP_FOREACH_6
#undef _REFLECT_PP_FOREACH_7
#undef _REFLECT_PP_FOREACH_8
#undef _REFLECT_PP_FOREACH_9
#undef _REFLECT_PP_FOREACH_10
#undef _REFLECT_PP_FOREACH_11
#undef _REFLECT_PP_FOREACH_12
#undef _REFLECT_PP_FOREACH_13
#undef _REFLECT_PP_FOREACH_14
#undef _REFLECT_PP_FOREACH_15
#undef _REFLECT_PP_FOREACH_16
#undef _REFLECT_PP_FOREACH_17
#undef _REFLECT_PP_FOREACH_18
#undef _REFLECT_PP_FOREACH_19
#undef _REFLECT_PP_FOREACH_20
#undef _REFLECT_PP_FOREACH_21
#undef _REFLECT_PP_FOREACH_22
#undef _REFLECT_PP_FOREACH_23
#undef _REFLECT_PP_FOREACH_24
#undef _REFLECT_PP_FOREACH_25
#undef _REFLECT_PP_FOREACH_26
#undef _REFLECT_PP_FOREACH_27
#undef _REFLECT_PP_FOREACH_28
#undef _REFLECT_PP_FOREACH_29
#undef _REFLECT_PP_FOREACH_30
#undef _REFLECT_PP_FOREACH_31
#undef _REFLECT_PP_FOREACH_32
#undef _REFLECT_PP_FOREACH_33
#undef _REFLECT_PP_FOREACH_34
#undef _REFLECT_PP_FOREACH_35
#undef _REFLECT_PP_FOREACH_36
#undef _REFLECT_PP_FOREACH_37
#undef _REFLECT_PP_FOREACH_38
#undef _REFLECT_PP_FOREACH_39
#undef _REFLECT_PP_FOREACH_40
#undef _REFLECT_PP_FOREACH_41
#undef _REFLECT_PP_FOREACH_42
#undef _REFLECT_PP_FOREACH_43
#undef _REFLECT_PP_FOREACH_44
#undef _REFLECT_PP_FOREACH_45
#undef _REFLECT_PP_FOREACH_46
#undef _REFLECT_PP_FOREACH_47
#undef _REFLECT_PP_FOREACH_48
#undef _REFLECT_PP_FOREACH_49
#undef _REFLECT_PP_FOREACH_50
#undef _REFLECT_PP_FOREACH_51
#undef _REFLECT_PP_FOREACH_52
#undef _REFLECT_PP_FOREACH_53
#undef _REFLECT_PP_FOREACH_54
#undef _REFLECT_PP_FOREACH_55
#undef _REFLECT_PP_FOREACH_56
#undef _REFLECT_PP_FOREACH_57
#undef _REFLECT_PP_FOREACH_58
#undef _REFLECT_PP_FOREACH_59
#undef _REFLECT_PP_FOREACH_60
#undef _REFLECT_PP_FOREACH_61
#undef _REFLECT_PP_FOREACH_62
#undef _REFLECT_PP_FOREACH_63
#undef _REFLECT_PP_FOREACH_64

#undef _GET_LAST_ARG_1
#undef _GET_LAST_ARG_2
#undef _GET_LAST_ARG_3
#undef _GET_LAST_ARG_4
#undef _GET_LAST_ARG_5
#undef _GET_LAST_ARG_6
#undef _GET_LAST_ARG_7
#undef _GET_LAST_ARG_8
#undef _GET_LAST_ARG_9
#undef _GET_LAST_ARG_10
#undef _GET_LAST_ARG_11
#undef _GET_LAST_ARG_12
#undef _GET_LAST_ARG_13
#undef _GET_LAST_ARG_14
#undef _GET_LAST_ARG_15
#undef _GET_LAST_ARG_16
#undef _GET_LAST_ARG_17
#undef _GET_LAST_ARG_18
#undef _GET_LAST_ARG_19
#undef _GET_LAST_ARG_20
#undef _GET_LAST_ARG_21
#undef _GET_LAST_ARG_22
#undef _GET_LAST_ARG_23
#undef _GET_LAST_ARG_24
#undef _GET_LAST_ARG_25
#undef _GET_LAST_ARG_26
#undef _GET_LAST_ARG_27
#undef _GET_LAST_ARG_28
#undef _GET_LAST_ARG_29
#undef _GET_LAST_ARG_30
#undef _GET_LAST_ARG_31
#undef _GET_LAST_ARG_32
#undef _GET_LAST_ARG_33
#undef _GET_LAST_ARG_34
#undef _GET_LAST_ARG_35
#undef _GET_LAST_ARG_36
#undef _GET_LAST_ARG_37
#undef _GET_LAST_ARG_38
#undef _GET_LAST_ARG_39
#undef _GET_LAST_ARG_40
#undef _GET_LAST_ARG_41
#undef _GET_LAST_ARG_42
#undef _GET_LAST_ARG_43
#undef _GET_LAST_ARG_44
#undef _GET_LAST_ARG_45
#undef _GET_LAST_ARG_46
#undef _GET_LAST_ARG_47
#undef _GET_LAST_ARG_48
#undef _GET_LAST_ARG_49
#undef _GET_LAST_ARG_50
#undef _GET_LAST_ARG_51
#undef _GET_LAST_ARG_52
#undef _GET_LAST_ARG_53
#undef _GET_LAST_ARG_54
#undef _GET_LAST_ARG_55
#undef _GET_LAST_ARG_56
#undef _GET_LAST_ARG_57
#undef _GET_LAST_ARG_58
#undef _GET_LAST_ARG_59
#undef _GET_LAST_ARG_60
#undef _GET_LAST_ARG_61
#undef _GET_LAST_ARG_62
#undef _GET_LAST_ARG_63
#undef _GET_LAST_ARG_64

#undef _DEL_LAST_ARG_1
#undef _DEL_LAST_ARG_2
#undef _DEL_LAST_ARG_3
#undef _DEL_LAST_ARG_4
#undef _DEL_LAST_ARG_5
#undef _DEL_LAST_ARG_6
#undef _DEL_LAST_ARG_7
#undef _DEL_LAST_ARG_8
#undef _DEL_LAST_ARG_9
#undef _DEL_LAST_ARG_10
#undef _DEL_LAST_ARG_11
#undef _DEL_LAST_ARG_12
#undef _DEL_LAST_ARG_13
#undef _DEL_LAST_ARG_14
#undef _DEL_LAST_ARG_15
#undef _DEL_LAST_ARG_16
#undef _DEL_LAST_ARG_17
#undef _DEL_LAST_ARG_18
#undef _DEL_LAST_ARG_19
#undef _DEL_LAST_ARG_20
#undef _DEL_LAST_ARG_21
#undef _DEL_LAST_ARG_22
#undef _DEL_LAST_ARG_23
#undef _DEL_LAST_ARG_24
#undef _DEL_LAST_ARG_25
#undef _DEL_LAST_ARG_26
#undef _DEL_LAST_ARG_27
#undef _DEL_LAST_ARG_28
#undef _DEL_LAST_ARG_29
#undef _DEL_LAST_ARG_30
#undef _DEL_LAST_ARG_31
#undef _DEL_LAST_ARG_32
#undef _DEL_LAST_ARG_33
#undef _DEL_LAST_ARG_34
#undef _DEL_LAST_ARG_35
#undef _DEL_LAST_ARG_36
#undef _DEL_LAST_ARG_37
#undef _DEL_LAST_ARG_38
#undef _DEL_LAST_ARG_39
#undef _DEL_LAST_ARG_40
#undef _DEL_LAST_ARG_41
#undef _DEL_LAST_ARG_42
#undef _DEL_LAST_ARG_43
#undef _DEL_LAST_ARG_44
#undef _DEL_LAST_ARG_45
#undef _DEL_LAST_ARG_46
#undef _DEL_LAST_ARG_47
#undef _DEL_LAST_ARG_48
#undef _DEL_LAST_ARG_49
#undef _DEL_LAST_ARG_50
#undef _DEL_LAST_ARG_51
#undef _DEL_LAST_ARG_52
#undef _DEL_LAST_ARG_53
#undef _DEL_LAST_ARG_54
#undef _DEL_LAST_ARG_55
#undef _DEL_LAST_ARG_56
#undef _DEL_LAST_ARG_57
#undef _DEL_LAST_ARG_58
#undef _DEL_LAST_ARG_59
#undef _DEL_LAST_ARG_60
#undef _DEL_LAST_ARG_61
#undef _DEL_LAST_ARG_62
#undef _DEL_LAST_ARG_63
#undef _DEL_LAST_ARG_64

#undef _REFLECT_PP_FOREACH_SIZEOF_IMPL
#undef _REFLECT_PP_FOREACH_SIZEOF
/**
 * @brief 同理, 更多可以通过以下生成:
import sys

n = int(sys.argv[1])

# 调用辅助宏
for i in range(1, n+1):
    print("#undef _REFLECT_PP_FOREACH_{0}".format(i))

print("#undef _REFLECT_PP_FOREACH_SIZEOF_IMPL")
print("#undef _REFLECT_PP_FOREACH_SIZEOF")

# 获取最后一个参数
for i in range(1, n+1):
    print("#undef _GET_LAST_ARG_{0}".format(i))

# 删除最后一个参数
for i in range(1, n+1):
    print("#undef _DEL_LAST_ARG_{0}".format(i))
 */

#undef _REFLECT_TO_JSON
#undef _REFLECT_PP_CALL_
#undef _REFLECT_PP_CALL
#undef _REFLECT_PP_FOREACH
#undef _REFLECT_CONSTRUCTOR_TO_JSON
#undef _REFLECT_CONSTRUCTOR_ARG
#undef _REFLECT_CONSTRUCTOR_ARG_END
#undef _REFLECT_CONSTRUCTOR_SET_ARG
#undef _REFLECT_CONSTRUCTOR_SET_ARG_END

#undef _GET_LAST_ARG
#undef _DEL_LAST_ARG

#undef REFLECT
#undef REFLECT_CONSTRUCTOR
#undef REFLECT_CONSTRUCTOR_ALL

#endif // !_HX_UN_REFLECT_JSON_H_