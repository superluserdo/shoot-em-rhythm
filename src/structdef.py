# -*- coding: utf-8 -*-
#
# TARGET arch is: []
# WORD_SIZE is: 8
# POINTER_SIZE is: 8
# LONGDOUBLE_SIZE is: 16
#
import ctypes


# if local wordsize is same as target, keep ctypes pointer function.
if ctypes.sizeof(ctypes.c_void_p) == 8:
    POINTER_T = ctypes.POINTER
else:
    # required to access _ctypes
    import _ctypes
    # Emulate a pointer class using the approriate c_int32/c_int64 type
    # The new class should have :
    # ['__module__', 'from_param', '_type_', '__dict__', '__weakref__', '__doc__']
    # but the class should be submitted to a unique instance for each base type
    # to that if A == B, POINTER_T(A) == POINTER_T(B)
    ctypes._pointer_t_type_cache = {}
    def POINTER_T(pointee):
        # a pointer should have the same length as LONG
        fake_ptr_base_type = ctypes.c_uint64 
        # specific case for c_void_p
        if pointee is None: # VOID pointer type. c_void_p.
            pointee = type(None) # ctypes.c_void_p # ctypes.c_ulong
            clsname = 'c_void'
        else:
            clsname = pointee.__name__
        if clsname in ctypes._pointer_t_type_cache:
            return ctypes._pointer_t_type_cache[clsname]
        # make template
        class _T(_ctypes._SimpleCData,):
            _type_ = 'L'
            _subtype_ = pointee
            def _sub_addr_(self):
                return self.value
            def __repr__(self):
                return '%s(%d)'%(clsname, self.value)
            def contents(self):
                raise TypeError('This is not a ctypes pointer.')
            def __init__(self, **args):
                raise TypeError('This is not a ctypes pointer. It is not instanciable.')
        _class = type('LP_%d_%s'%(8, clsname), (_T,),{}) 
        ctypes._pointer_t_type_cache[clsname] = _class
        return _class

c_int128 = ctypes.c_ubyte*16
c_uint128 = c_int128
void = None
if ctypes.sizeof(ctypes.c_longdouble) == 16:
    c_long_double_t = ctypes.c_longdouble
else:
    c_long_double_t = ctypes.c_ubyte*16



class struct_SDL_Point(ctypes.Structure):
    _pack_ = True # source:False
    _fields_ = [
    ('x', ctypes.c_int32),
    ('y', ctypes.c_int32),
     ]

class struct_SDL_Rect(ctypes.Structure):
    _pack_ = True # source:False
    _fields_ = [
    ('x', ctypes.c_int32),
    ('y', ctypes.c_int32),
    ('w', ctypes.c_int32),
    ('h', ctypes.c_int32),
     ]


# values for enumeration 'c__EA_SDL_RendererFlip'
SDL_FLIP_NONE = 0
SDL_FLIP_HORIZONTAL = 1
SDL_FLIP_VERTICAL = 2
c__EA_SDL_RendererFlip = ctypes.c_int # enum
class struct_SDL_Renderer(ctypes.Structure):
    pass

class struct_SDL_Texture(ctypes.Structure):
    pass

class union_c__UA_pthread_mutex_t(ctypes.Union):
    pass

class struct___pthread_mutex_s(ctypes.Structure):
    pass

class struct___pthread_internal_list(ctypes.Structure):
    pass

struct___pthread_internal_list._pack_ = True # source:False
struct___pthread_internal_list._fields_ = [
    ('__prev', POINTER_T(struct___pthread_internal_list)),
    ('__next', POINTER_T(struct___pthread_internal_list)),
]

struct___pthread_mutex_s._pack_ = True # source:False
struct___pthread_mutex_s._fields_ = [
    ('__lock', ctypes.c_int32),
    ('__count', ctypes.c_uint32),
    ('__owner', ctypes.c_int32),
    ('__nusers', ctypes.c_uint32),
    ('__kind', ctypes.c_int32),
    ('__spins', ctypes.c_int16),
    ('__elision', ctypes.c_int16),
    ('__list', struct___pthread_internal_list),
]

union_c__UA_pthread_mutex_t._pack_ = True # source:False
union_c__UA_pthread_mutex_t._fields_ = [
    ('__data', struct___pthread_mutex_s),
    ('__size', ctypes.c_char * 40),
    ('__align', ctypes.c_int64),
    ('PADDING_0', ctypes.c_ubyte * 32),
]

class struct_xy_struct(ctypes.Structure):
    _pack_ = True # source:False
    _fields_ = [
    ('x', ctypes.c_int32),
    ('y', ctypes.c_int32),
     ]

grid = struct_xy_struct # Variable struct_xy_struct
class struct_size_ratio_struct(ctypes.Structure):
    _pack_ = True # source:False
    _fields_ = [
    ('w', ctypes.c_float),
    ('h', ctypes.c_float),
     ]

class struct_std(ctypes.Structure):
    pass

class struct_visual_container_struct(ctypes.Structure):
    pass


# values for enumeration 'aspctr_lock_e'
WH_INDEPENDENT = 0
W_DOMINANT = 1
H_DOMINANT = 2
aspctr_lock_e = ctypes.c_int # enum
class struct_float_rect(ctypes.Structure):
    _pack_ = True # source:False
    _fields_ = [
    ('x', ctypes.c_double),
    ('y', ctypes.c_double),
    ('w', ctypes.c_double),
    ('h', ctypes.c_double),
     ]

struct_visual_container_struct._pack_ = True # source:False
struct_visual_container_struct._fields_ = [
    ('inherit', POINTER_T(struct_visual_container_struct)),
    ('rect', struct_float_rect),
    ('aspctr', ctypes.c_float),
    ('aspctr_lock', aspctr_lock_e),
]

class struct_animate_specific(ctypes.Structure):
    pass

class struct_animate_generic(ctypes.Structure):
    pass

class struct_clip(ctypes.Structure):
    pass

class struct_frame(ctypes.Structure):
    _pack_ = True # source:False
    _fields_ = [
    ('rect', struct_SDL_Rect),
    ('anchor_hook', struct_xy_struct),
    ('duration', ctypes.c_float),
     ]


# values for enumeration 'scale_mode_e'
WIDTH = 0
HEIGHT = 1
scale_mode_e = ctypes.c_int # enum
struct_clip._pack_ = True # source:False
struct_clip._fields_ = [
    ('img', POINTER_T(struct_SDL_Texture)),
    ('num_frames', ctypes.c_int32),
    ('PADDING_0', ctypes.c_ubyte * 4),
    ('frames', POINTER_T(struct_frame)),
    ('container_scale_factor', ctypes.c_float),
    ('container_scale_mode', scale_mode_e),
]

struct_animate_generic._pack_ = True # source:False
struct_animate_generic._fields_ = [
    ('num_clips', ctypes.c_int32),
    ('PADDING_0', ctypes.c_ubyte * 4),
    ('clips', POINTER_T(POINTER_T(struct_clip))),
    ('default_specific', POINTER_T(struct_animate_specific)),
]

class struct_render_node(ctypes.Structure):
    pass

class struct_func_node(ctypes.Structure):
    pass

struct_func_node._pack_ = True # source:False
struct_func_node._fields_ = [
    ('data', POINTER_T(None)),
    ('func', POINTER_T(ctypes.CFUNCTYPE(None, POINTER_T(None), POINTER_T(None)))),
    ('next', POINTER_T(struct_func_node)),
]

struct_render_node._pack_ = True # source:False
struct_render_node._fields_ = [
    ('prev', POINTER_T(struct_render_node)),
    ('next', POINTER_T(struct_render_node)),
    ('rect_in', POINTER_T(struct_SDL_Rect)),
    ('rect_out', struct_SDL_Rect),
    ('img', POINTER_T(struct_SDL_Texture)),
    ('customRenderFunc', POINTER_T(ctypes.CFUNCTYPE(ctypes.c_int32, POINTER_T(None)))),
    ('customRenderArgs', POINTER_T(None)),
    ('renderer', POINTER_T(struct_SDL_Renderer)),
    ('animation', POINTER_T(struct_animate_specific)),
    ('transform_list', POINTER_T(struct_func_node)),
    ('z', ctypes.c_float),
    ('PADDING_0', ctypes.c_ubyte * 4),
]


# values for enumeration 'layer_mode_e'
TIGHT = 0
GLOBAL = 1
layer_mode_e = ctypes.c_int # enum
class struct_anchor_struct(ctypes.Structure):
    _pack_ = True # source:False
    _fields_ = [
    ('pos_anim_internal', struct_size_ratio_struct),
    ('anim', POINTER_T(struct_animate_specific)),
     ]

class struct_rule_node(ctypes.Structure):
    pass

struct_rule_node._pack_ = True # source:False
struct_rule_node._fields_ = [
    ('rule', POINTER_T(ctypes.CFUNCTYPE(None, POINTER_T(None)))),
    ('data', POINTER_T(None)),
    ('next', POINTER_T(struct_rule_node)),
]

struct_animate_specific._pack_ = True # source:False
struct_animate_specific._fields_ = [
    ('generic', POINTER_T(struct_animate_generic)),
    ('rules_list', POINTER_T(struct_rule_node)),
    ('clip', ctypes.c_int32),
    ('frame', ctypes.c_int32),
    ('speed', ctypes.c_float),
    ('loops', ctypes.c_int32),
    ('return_clip', ctypes.c_int32),
    ('lastFrameBeat', ctypes.c_float),
    ('native_offset', struct_xy_struct),
    ('parent', POINTER_T(struct_std)),
    ('container_scale_factor', ctypes.c_float),
    ('container_scale_mode', scale_mode_e),
    ('layer_mode', layer_mode_e),
    ('z', ctypes.c_float),
    ('screen_height_ratio', ctypes.c_float),
    ('PADDING_0', ctypes.c_ubyte * 4),
    ('anchor_grabbed', POINTER_T(struct_anchor_struct)),
    ('anchors_exposed', POINTER_T(struct_anchor_struct)),
    ('transform_list', POINTER_T(struct_func_node)),
    ('render_node', POINTER_T(struct_render_node)),
    ('rect_out', struct_SDL_Rect),
    ('rect_out_container_scale', struct_float_rect),
    ('next', POINTER_T(struct_animate_specific)),
]

struct_std._pack_ = True # source:False
struct_std._fields_ = [
    ('name', POINTER_T(ctypes.c_char)),
    ('pos', struct_xy_struct),
    ('size_ratio', struct_size_ratio_struct),
    ('container', POINTER_T(struct_visual_container_struct)),
    ('animation', POINTER_T(struct_animate_specific)),
    ('self', POINTER_T(None)),
]

class struct_living(ctypes.Structure):
    _pack_ = True # source:False
    _fields_ = [
    ('HP', ctypes.c_int32),
    ('power', ctypes.c_int32),
    ('defence', ctypes.c_float),
    ('max_HP', ctypes.c_int32),
    ('max_PP', ctypes.c_int32),
    ('invincibility', ctypes.c_int32),
    ('invincibility_toggle', ctypes.c_int32),
    ('PADDING_0', ctypes.c_ubyte * 4),
    ('self', POINTER_T(None)),
     ]

class struct_std_list(ctypes.Structure):
    pass

struct_std_list._pack_ = True # source:False
struct_std_list._fields_ = [
    ('std', POINTER_T(struct_std)),
    ('next', POINTER_T(struct_std_list)),
    ('prev', POINTER_T(struct_std_list)),
]

class struct_laser_struct(ctypes.Structure):
    _pack_ = True # source:False
    _fields_ = [
    ('power', ctypes.c_int32),
    ('count', ctypes.c_int32),
    ('on', ctypes.c_int32),
    ('turnon', ctypes.c_int32),
    ('turnoff', ctypes.c_int32),
     ]

class struct_sword_struct(ctypes.Structure):
    pass

class union_sword_struct_0(ctypes.Union):
    pass

class struct_sword_struct_0_0(ctypes.Structure):
    _pack_ = True # source:False
    _fields_ = [
    ('name', POINTER_T(ctypes.c_char)),
    ('pos', struct_xy_struct),
    ('size_ratio', struct_size_ratio_struct),
    ('container', POINTER_T(struct_visual_container_struct)),
    ('animation', POINTER_T(struct_animate_specific)),
    ('self', POINTER_T(None)),
     ]

union_sword_struct_0._pack_ = True # source:False
union_sword_struct_0._fields_ = [
    ('_0', struct_sword_struct_0_0),
    ('std', struct_std),
]

struct_sword_struct._pack_ = True # source:False
struct_sword_struct._fields_ = [
    ('power', ctypes.c_int32),
    ('count', ctypes.c_int32),
    ('down', ctypes.c_int32),
    ('swing', ctypes.c_int32),
    ('rect_in', struct_SDL_Rect),
    ('rect_out', struct_SDL_Rect),
    ('_6', union_sword_struct_0),
]

class struct_status_struct(ctypes.Structure):
    pass

class struct_level_struct(ctypes.Structure):
    pass

class struct_lane_struct(ctypes.Structure):
    _pack_ = True # source:False
    _fields_ = [
    ('total', ctypes.c_int32),
    ('currentlane', ctypes.c_int32),
    ('lanewidth', ctypes.c_float),
    ('PADDING_0', ctypes.c_ubyte * 4),
    ('laneheight', POINTER_T(ctypes.c_float)),
    ('containers', POINTER_T(struct_visual_container_struct)),
     ]

class struct_monster(ctypes.Structure):
    _pack_ = True # source:False
    _fields_ = [
    ('health', ctypes.c_int32),
    ('attack', ctypes.c_int32),
    ('defence', ctypes.c_float),
    ('Src', ctypes.c_int32 * 2),
    ('wh', ctypes.c_int32 * 2),
    ('generic_bank_index', ctypes.c_int32),
    ('image', POINTER_T(struct_SDL_Texture)),
     ]

class struct_item(ctypes.Structure):
    _pack_ = True # source:False
    _fields_ = [
    ('itemnumber', ctypes.c_int32),
    ('PADDING_0', ctypes.c_ubyte * 4),
    ('int1', POINTER_T(ctypes.c_int32)),
    ('int2', ctypes.c_int32),
    ('PADDING_1', ctypes.c_ubyte * 4),
    ('otherdata', POINTER_T(None)),
    ('functionptr', POINTER_T(None)),
    ('Src', ctypes.c_int32 * 2),
    ('wh', ctypes.c_int32 * 2),
    ('image', POINTER_T(POINTER_T(struct_SDL_Texture))),
     ]

class struct_level_effects_struct(ctypes.Structure):
    _pack_ = True # source:False
    _fields_ = [
    ('angle', ctypes.c_double),
    ('colournum', ctypes.c_int32),
    ('hue', ctypes.c_int32),
     ]

class struct_rects_struct(ctypes.Structure):
    _pack_ = True # source:False
    _fields_ = [
    ('rcLaser', struct_SDL_Rect * 3),
    ('rcLaserSrc', struct_SDL_Rect * 3),
    ('rcScore', struct_SDL_Rect * 5),
    ('rcScoreSrc', struct_SDL_Rect * 5),
    ('rcBeat', struct_SDL_Rect * 5),
    ('rcBeatSrc', struct_SDL_Rect * 5),
     ]

class struct_level_var_struct(ctypes.Structure):
    pass

class struct_mutex_list_struct(ctypes.Structure):
    _pack_ = True # source:False
    _fields_ = [
    ('soundstatus_mutex', union_c__UA_pthread_mutex_t),
     ]

struct_level_var_struct._pack_ = True # source:False
struct_level_var_struct._fields_ = [
    ('mutexes', POINTER_T(struct_mutex_list_struct)),
    ('soundstatus', ctypes.c_int32),
    ('directionbuttonlist', ctypes.c_int32 * 4),
    ('history', ctypes.c_int32 * 4),
    ('histwrite', ctypes.c_int32),
    ('histread', ctypes.c_int32),
    ('actionbuttonlist', ctypes.c_int32 * 4),
    ('acthistory', ctypes.c_int32 * 4),
    ('acthistwrite', ctypes.c_int32),
    ('acthistread', ctypes.c_int32),
    ('PADDING_0', ctypes.c_ubyte * 4),
]

struct_level_struct._pack_ = True # source:False
struct_level_struct._fields_ = [
    ('score', ctypes.c_int32),
    ('gameover', ctypes.c_int32),
    ('levelover', ctypes.c_int32),
    ('pauselevel', ctypes.c_int32),
    ('currentlevel', ctypes.c_int32),
    ('grid', struct_xy_struct),
    ('maxscreens', ctypes.c_int32),
    ('totalnativedist', ctypes.c_int32),
    ('partymode', ctypes.c_int32),
    ('speedmult', ctypes.c_float),
    ('speedmultmon', ctypes.c_float),
    ('currentscreen', ctypes.c_int32),
    ('PADDING_0', ctypes.c_ubyte * 4),
    ('laneheight', POINTER_T(ctypes.c_float)),
    ('lanes', struct_lane_struct),
    ('object_list_stack', POINTER_T(struct_std_list)),
    ('laser', struct_laser_struct),
    ('PADDING_1', ctypes.c_ubyte * 4),
    ('sword', struct_sword_struct),
    ('vars', POINTER_T(struct_level_var_struct)),
    ('effects', POINTER_T(struct_level_effects_struct)),
    ('rects', POINTER_T(struct_rects_struct)),
    ('bestiary', POINTER_T(struct_monster) * 10),
    ('itempokedex', POINTER_T(struct_item) * 10),
    ('itemscreenstrip', POINTER_T(POINTER_T(ctypes.c_int32 * 2 * 20 * 5))),
    ('remainder', POINTER_T(ctypes.c_double)),
]

class struct_audio_struct(ctypes.Structure):
    _pack_ = True # source:False
    _fields_ = [
    ('track', ctypes.c_int32),
    ('newtrack', ctypes.c_int32),
    ('noise', ctypes.c_int32),
    ('soundchecklist', ctypes.c_int32 * 10),
    ('music_mute', ctypes.c_int32),
    ('music_volume', ctypes.c_float),
     ]

class struct_program_struct(ctypes.Structure):
    pass

class struct_hooktypes_struct(ctypes.Structure):
    pass

class struct_hooks_list_struct(ctypes.Structure):
    pass

struct_hooks_list_struct._pack_ = True # source:False
struct_hooks_list_struct._fields_ = [
    ('hookfunc', POINTER_T(ctypes.CFUNCTYPE(POINTER_T(None), POINTER_T(struct_status_struct)))),
    ('next', POINTER_T(struct_hooks_list_struct)),
]

struct_hooktypes_struct._pack_ = True # source:False
struct_hooktypes_struct._fields_ = [
    ('frame', POINTER_T(struct_hooks_list_struct)),
    ('level_init', POINTER_T(struct_hooks_list_struct)),
    ('level_loop', POINTER_T(struct_hooks_list_struct)),
]

struct_program_struct._pack_ = True # source:False
struct_program_struct._fields_ = [
    ('hooks', struct_hooktypes_struct),
    ('python_helper_function', POINTER_T(None)),
    ('status_python_capsule', POINTER_T(None)),
    ('python_interpreter_activate', ctypes.c_int32),
    ('python_interpreter_enable', ctypes.c_int32),
]

class struct_time_struct(ctypes.Structure):
    _pack_ = True # source:False
    _fields_ = [
    ('ticks', ctypes.c_int32),
    ('ticks_last_frame', ctypes.c_int32),
    ('countbeats', ctypes.c_int32),
    ('bps', ctypes.c_float),
    ('startbeat', ctypes.c_float),
    ('currentbeat', ctypes.c_float),
    ('currentbeat_int', ctypes.c_int32),
    ('pxperbeat', ctypes.c_float),
    ('framecount', ctypes.c_int32),
    ('fpsanim', ctypes.c_int32),
    ('fpsglobal', ctypes.c_int32),
    ('PADDING_0', ctypes.c_ubyte * 4),
    ('pauselevel', POINTER_T(ctypes.c_int32)),
    ('pause_change', ctypes.c_int32),
    ('zerotime', ctypes.c_int32),
    ('pausetime', ctypes.c_int32),
    ('pausetime_completed', ctypes.c_int32),
    ('pausetime_ongoing', ctypes.c_int32),
    ('startpause', ctypes.c_int32),
    ('endpause', ctypes.c_int32),
    ('intervalanim', ctypes.c_float),
    ('intervalglobal', ctypes.c_float),
    ('PADDING_1', ctypes.c_ubyte * 4),
     ]

class struct_player_struct(ctypes.Structure):
    pass

class union_player_struct_0(ctypes.Union):
    pass

class struct_player_struct_0_0(ctypes.Structure):
    _pack_ = True # source:False
    _fields_ = [
    ('name', POINTER_T(ctypes.c_char)),
    ('pos', struct_xy_struct),
    ('size_ratio', struct_size_ratio_struct),
    ('container', POINTER_T(struct_visual_container_struct)),
    ('animation', POINTER_T(struct_animate_specific)),
    ('self', POINTER_T(None)),
     ]

union_player_struct_0._pack_ = True # source:False
union_player_struct_0._fields_ = [
    ('_0', struct_player_struct_0_0),
    ('std', struct_std),
]

struct_player_struct._pack_ = True # source:False
struct_player_struct._fields_ = [
    ('_0', union_player_struct_0),
    ('invinciblecounter', ctypes.c_int32 * 2),
    ('sword', ctypes.c_int32),
    ('direction', ctypes.c_int32),
    ('flydir', ctypes.c_int32),
    ('PADDING_0', ctypes.c_ubyte * 4),
    ('living', struct_living),
]

class struct_graphics_struct(ctypes.Structure):
    pass

class struct_rendercopyex_struct(ctypes.Structure):
    _pack_ = True # source:False
    _fields_ = [
    ('renderer', POINTER_T(struct_SDL_Renderer)),
    ('texture', POINTER_T(struct_SDL_Texture)),
    ('srcrect', POINTER_T(struct_SDL_Rect)),
    ('dstrect', POINTER_T(struct_SDL_Rect)),
    ('angle', ctypes.c_double),
    ('center', POINTER_T(struct_SDL_Point)),
    ('flip', c__EA_SDL_RendererFlip),
    ('PADDING_0', ctypes.c_ubyte * 4),
     ]

class struct_texture_struct(ctypes.Structure):
    _pack_ = True # source:False
    _fields_ = [
    ('Spriteimg', POINTER_T(struct_SDL_Texture)),
    ('Laserimg', POINTER_T(struct_SDL_Texture)),
    ('Swordimg', POINTER_T(struct_SDL_Texture)),
    ('Timg', POINTER_T(struct_SDL_Texture)),
    ('Mon0img', POINTER_T(struct_SDL_Texture)),
    ('Mon1img', POINTER_T(struct_SDL_Texture)),
    ('Scoreimg', POINTER_T(struct_SDL_Texture)),
    ('Beatimg', POINTER_T(struct_SDL_Texture)),
    ('Itemimg', POINTER_T(struct_SDL_Texture)),
    ('texTarget', POINTER_T(struct_SDL_Texture)),
     ]

class struct_ui_struct(ctypes.Structure):
    pass

class struct_ui_bar(ctypes.Structure):
    pass

class union_ui_bar_0(ctypes.Union):
    pass

class struct_ui_bar_0_0(ctypes.Structure):
    _pack_ = True # source:False
    _fields_ = [
    ('name', POINTER_T(ctypes.c_char)),
    ('pos', struct_xy_struct),
    ('size_ratio', struct_size_ratio_struct),
    ('container', POINTER_T(struct_visual_container_struct)),
    ('animation', POINTER_T(struct_animate_specific)),
    ('self', POINTER_T(None)),
     ]

union_ui_bar_0._pack_ = True # source:False
union_ui_bar_0._fields_ = [
    ('_0', struct_ui_bar_0_0),
    ('std', struct_std),
]

struct_ui_bar._pack_ = True # source:False
struct_ui_bar._fields_ = [
    ('amount', POINTER_T(ctypes.c_int32)),
    ('max', POINTER_T(ctypes.c_int32)),
    ('_2', union_ui_bar_0),
]

class struct_ui_counter(ctypes.Structure):
    pass

class union_ui_counter_0(ctypes.Union):
    pass

class struct_ui_counter_0_0(ctypes.Structure):
    _pack_ = True # source:False
    _fields_ = [
    ('name', POINTER_T(ctypes.c_char)),
    ('pos', struct_xy_struct),
    ('size_ratio', struct_size_ratio_struct),
    ('container', POINTER_T(struct_visual_container_struct)),
    ('animation', POINTER_T(struct_animate_specific)),
    ('self', POINTER_T(None)),
     ]

union_ui_counter_0._pack_ = True # source:False
union_ui_counter_0._fields_ = [
    ('_0', struct_ui_counter_0_0),
    ('std', struct_std),
]

struct_ui_counter._pack_ = True # source:False
struct_ui_counter._fields_ = [
    ('value', POINTER_T(ctypes.c_int32)),
    ('digits', ctypes.c_int32),
    ('PADDING_0', ctypes.c_ubyte * 4),
    ('array', POINTER_T(ctypes.c_int32)),
    ('_3', union_ui_counter_0),
]

struct_ui_struct._pack_ = True # source:False
struct_ui_struct._fields_ = [
    ('power', struct_ui_bar),
    ('hp', struct_ui_bar),
    ('score', struct_ui_counter),
    ('beat', struct_ui_counter),
]

struct_graphics_struct._pack_ = True # source:False
struct_graphics_struct._fields_ = [
    ('width', ctypes.c_int32),
    ('height', ctypes.c_int32),
    ('screen', struct_visual_container_struct),
    ('renderer', POINTER_T(struct_SDL_Renderer)),
    ('render_node_head', POINTER_T(struct_render_node)),
    ('render_node_tail', POINTER_T(struct_render_node)),
    ('ui', POINTER_T(struct_ui_struct)),
    ('imgs', POINTER_T(struct_texture_struct)),
    ('num_images', ctypes.c_int32),
    ('PADDING_0', ctypes.c_ubyte * 4),
    ('rendercopyex_data', POINTER_T(struct_rendercopyex_struct)),
    ('image_bank', POINTER_T(POINTER_T(struct_SDL_Texture))),
]

struct_status_struct._pack_ = True # source:False
struct_status_struct._fields_ = [
    ('level', POINTER_T(struct_level_struct)),
    ('player', POINTER_T(struct_player_struct)),
    ('audio', POINTER_T(struct_audio_struct)),
    ('timing', POINTER_T(struct_time_struct)),
    ('graphics', POINTER_T(struct_graphics_struct)),
    ('program', POINTER_T(struct_program_struct)),
]

class struct_hooks_struct(ctypes.Structure):
    pass

struct_hooks_struct._pack_ = True # source:False
struct_hooks_struct._fields_ = [
    ('numhooks', ctypes.c_int32),
    ('PADDING_0', ctypes.c_ubyte * 4),
    ('hookfuncs', POINTER_T(POINTER_T(ctypes.CFUNCTYPE(POINTER_T(None), POINTER_T(struct_status_struct))))),
]


# values for enumeration 'visual_structure_name_e'
SCREEN = 0
LEVEL_UI_TOP = 1
LEVEL_PLAY_AREA = 2
visual_structure_name_e = ctypes.c_int # enum

# values for enumeration 'vector_e'
START = -3
ELEM_SIZE = -3
LEN = -2
USED = -1
DATA = 0
vector_e = ctypes.c_int # enum
class struct_monster_node(ctypes.Structure):
    pass

struct_monster_node._pack_ = True # source:False
struct_monster_node._fields_ = [
    ('montype', ctypes.c_char),
    ('status', ctypes.c_char),
    ('PADDING_0', ctypes.c_ubyte * 2),
    ('health', ctypes.c_int32),
    ('speed', ctypes.c_float),
    ('entrybeat', ctypes.c_float),
    ('remainder', ctypes.c_float),
    ('monster_rect', struct_SDL_Rect),
    ('monster_src', struct_SDL_Rect),
    ('PADDING_1', ctypes.c_ubyte * 4),
    ('animation', POINTER_T(struct_animate_specific)),
    ('next', POINTER_T(struct_monster_node)),
]

class struct_monster_new(ctypes.Structure):
    pass

class union_monster_new_0(ctypes.Union):
    pass

class struct_monster_new_0_0(ctypes.Structure):
    _pack_ = True # source:False
    _fields_ = [
    ('name', POINTER_T(ctypes.c_char)),
    ('pos', struct_xy_struct),
    ('size_ratio', struct_size_ratio_struct),
    ('container', POINTER_T(struct_visual_container_struct)),
    ('animation', POINTER_T(struct_animate_specific)),
    ('self', POINTER_T(None)),
     ]

union_monster_new_0._pack_ = True # source:False
union_monster_new_0._fields_ = [
    ('_0', struct_monster_new_0_0),
    ('std', struct_std),
]

struct_monster_new._pack_ = True # source:False
struct_monster_new._fields_ = [
    ('_0', union_monster_new_0),
    ('living', struct_living),
]


# values for enumeration 'graphic_cat_e'
CHARACTER = 0
UI = 1
UI_BAR = 2
UI_COUNTER = 3
graphic_cat_e = ctypes.c_int # enum

# values for enumeration 'graphic_type_e'
PLAYER = 0
FLYING_HAMSTER = 1
HP = 2
POWER = 3
COLOURED_BAR = 4
NUMBERS = 5
PLAYER2 = 6
SWORD = 7
SMILEY = 8
graphic_type_e = ctypes.c_int # enum

# values for enumeration 'return_codes_e'
R_SUCCESS = 0
R_FAILURE = 1
R_RESTART_LEVEL = 2
R_LOOP_LEVEL = 3
R_QUIT_TO_DESKTOP = 4
R_CASCADE_UP = 100
R_CASCADE_UP_MAX = 199
R_STARTSCREEN = 200
R_LEVELS = 201
return_codes_e = ctypes.c_int # enum

# values for enumeration 'hook_type_e'
FRAME = 0
LEVEL_INIT = 1
LEVEL_LOOP = 2
hook_type_e = ctypes.c_int # enum
__all__ = \
    ['struct_sword_struct', 'POWER', 'struct_monster_new_0_0',
    'R_SUCCESS', 'struct_xy_struct', 'struct_monster_node',
    'graphic_type_e', 'union_c__UA_pthread_mutex_t',
    'SDL_FLIP_HORIZONTAL', 'struct_program_struct',
    'struct_ui_counter', 'LEVEL_PLAY_AREA', 'struct_float_rect',
    'SWORD', 'R_STARTSCREEN', 'R_CASCADE_UP', 'SDL_FLIP_NONE', 'DATA',
    'struct_audio_struct', 'struct_texture_struct',
    'struct_mutex_list_struct', 'GLOBAL', 'struct_rule_node',
    'struct_anchor_struct', 'layer_mode_e', 'struct_lane_struct',
    'union_monster_new_0', 'HEIGHT', 'struct_hooktypes_struct',
    'c__EA_SDL_RendererFlip', 'struct_level_struct',
    'struct___pthread_mutex_s', 'SCREEN', 'PLAYER2', 'ELEM_SIZE',
    'R_FAILURE', 'scale_mode_e', 'grid', 'LEN', 'START',
    'struct_player_struct', 'UI', 'struct_living',
    'struct_animate_specific', 'union_sword_struct_0',
    'struct_time_struct', 'struct___pthread_internal_list',
    'struct_clip', 'return_codes_e', 'USED',
    'struct_hooks_list_struct', 'struct_size_ratio_struct',
    'struct_SDL_Texture', 'R_RESTART_LEVEL', 'UI_COUNTER', 'R_LEVELS',
    'HP', 'graphic_cat_e', 'struct_animate_generic',
    'struct_status_struct', 'struct_SDL_Rect', 'R_LOOP_LEVEL',
    'struct_frame', 'NUMBERS', 'struct_laser_struct',
    'struct_rendercopyex_struct', 'COLOURED_BAR', 'R_CASCADE_UP_MAX',
    'struct_monster', 'struct_player_struct_0_0', 'WH_INDEPENDENT',
    'UI_BAR', 'FRAME', 'struct_ui_counter_0_0', 'PLAYER',
    'struct_level_effects_struct', 'hook_type_e', 'WIDTH',
    'visual_structure_name_e', 'LEVEL_LOOP', 'struct_SDL_Renderer',
    'union_ui_bar_0', 'struct_visual_container_struct',
    'struct_render_node', 'CHARACTER', 'TIGHT', 'struct_std_list',
    'struct_graphics_struct', 'LEVEL_UI_TOP', 'union_ui_counter_0',
    'aspctr_lock_e', 'struct_rects_struct', 'SMILEY',
    'struct_ui_bar_0_0', 'LEVEL_INIT', 'struct_ui_bar', 'struct_item',
    'struct_ui_struct', 'H_DOMINANT', 'struct_std', 'vector_e',
    'R_QUIT_TO_DESKTOP', 'struct_monster_new', 'SDL_FLIP_VERTICAL',
    'struct_func_node', 'W_DOMINANT', 'struct_hooks_struct',
    'struct_level_var_struct', 'union_player_struct_0',
    'FLYING_HAMSTER', 'struct_SDL_Point', 'struct_sword_struct_0_0']
