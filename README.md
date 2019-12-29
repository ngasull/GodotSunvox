# GodotSunvox

Low-level Sunvox bindings for GDNative.

The idea behind these bindings follows the philosophy of Sunvox: do less, do simple things, but do them efficiently. The bindings don't change the C API except when idiomatically integrating to Godot's environment. For instance, functions that fill input buffers with data are wrapped to generate the data arrays themselves, which is more idiomatic to GDScript.

Concerning the [API documentation, please refer to the appropriate section below](#api).

## Plugin

These bindings can simply be accessed as a plugin. Once added to `addons` directory, don't forget to enable the plugin via `Project > Project Settings > Plugins` tab, and enable _Sunvox_.

The plugin uses AutoLoad to instanciate a `sunvox` singleton, from which the whole Sunvox API is available.

From your project, you need to initialize Sunvox:
```
func _ready():
  Sunvox.init("", 44100, 2, 0)
```

See API docs for advanced usage of `init()`.

If you'd like to init the plugin in regular playback mode and in callback mode at the same time (for wav streaming instead of letting Sunvox handle the playback), I'd recommend having two instances of the plugin at once: _Sunvox_ and _SunvoxCallback_. They each have their own copy of the libraries which guarantees their independence.


## Manually building

Python, scons and clang are required to build the library:
```bash
scons platform=linux use_llvm=yes
```

`linux` can be replaced by `osx` or `windows`.


## API

```c
/**
 * Open the first available slot
 * Returns the opened slot number
 */
int acquire_slot();

/**
 * Get the next piece of SunVox audio from the Output module.
 * With audio_callback() you can ignore the built-in SunVox sound output mechanism and use some other sound system.
 * SV_INIT_FLAG_USER_AUDIO_CALLBACK and SV_INIT_FLAG_AUDIO_INT16 ((1 << 1) & (1 << 2)) flag in init() must be set.
 * Parameters:
 *   frames - number of frames in destination buffer;
 *   latency - audio latency (in frames);
 *   out_time - buffer output time (in system ticks);
 * Example:
 *   user_out_time = ... ; //output time in user time space (NOT SunVox time space!)
 *   user_cur_time = ... ; //current time (user time space)
 *   user_ticks_per_second = ... ; //ticks per second (user time space)
 *   user_latency = user_out_time - use_cur_time; //latency in user time space
 *   unsigned int sunvox_latency = ( user_latency * get_ticks_per_second() ) / user_ticks_per_second; //latency in SunVox time space
 *   unsigned int latency_frames = ( user_latency * sample_rate_Hz ) / user_ticks_per_second; //latency in frames
 *   audio_callback( buf, frames, latency_frames, get_ticks() + sunvox_latency );
 * Returns the signal as bytes
 */
PoolByteArray audio_callback( int frames, int latency, int out_time );

/**
 * Send some data to the Input module and receive the filtered data from the Output module.
 * It's the same as audio_callback() but you also can specify the input buffer.
 * Parameters:
 *   ...
 *   in_type - input buffer type: 0 - signed short (16bit integer); 1 - float (32bit floating point);
 *   in_channels - number of input channels;
 *   in_buf - input buffer; stereo data will be interleaved in this buffer: LRLR... ; where the LR is the one frame (Left+Right channels);
 * Returns the signal as bytes
 */
PoolByteArray audio_callback2( int frames, int latency, int out_time, int in_type, int in_channels, PoolByteArray in_buf );

/**
 * Closes the given slot
 * Returns whatever Sunvox returns
 */
int close_slot( int slot );

/**
 * USE LOCK/UNLOCK!
 * Connect module source's output to module destination's input
 * Returns whatever Sunvox returns
 */
int connect_module( int slot, int source, int destination );

/**
 * De-initializes Sunvox
 * Returns whatever Sunvox returns
 */
int deinit();

/**
 * USE LOCK/UNLOCK!
 * Disconnect module source's output to module destination's input
 * Returns whatever Sunvox returns
 */
int disconnect_module( int slot, int source, int destination );

/**
 * Checks if the song has ended
 * Returns 0 - song is playing now; 1 - stopped
 */
int end_of_song( int slot );

/**
 * Get the line currently playing
 * Returns the line
 */
int get_current_line( int slot );

/**
 * Get the line currently playing
 * Returns the line with decimals
 */
float get_current_line2( int slot );

/**
 * Get current signal
 * Returns signal from 0 to 255
 */
int get_current_signal_level( int slot, int channel );

/**
 * Get latest Sunvox's logs, amounting given size
 * Returns size bytes of logs
 */
String get_log( int size );

/**
 * Get module color in the following format: 0xBBGGRR
 * Returns the color
 */
int get_module_color( int slot, int mod_num );

/**
 * Get the name of a module's controller
 * Returns the name
 */
int get_module_ctl_name( int slot, int mod_num, int ctl_num, );

/**
 * Get the value of a module's controller
 * Returns the value
 */
int get_module_ctl_value( int slot, int mod_num, int ctl_num );

/**
 * Get the flags of a module where
 * SV_MODULE_FLAG_EXISTS 1
 * SV_MODULE_FLAG_EFFECT 2
 * SV_MODULE_INPUTS_OFF 16
 * SV_MODULE_INPUTS_MASK ( 255 << SV_MODULE_INPUTS_OFF )
 * SV_MODULE_OUTPUTS_OFF ( 16 + 8 )
 * SV_MODULE_OUTPUTS_MASK ( 255 << SV_MODULE_OUTPUTS_OFF )
 * Returns the flags
 */
int get_module_flags( int slot, int mod_num );

/**
 * Get the inputs of a module
 * Returns the input module ids
 */
PoolIntArray get_module_inputs( int slot, int mod_num );

/**
 * Get the name of a module
 * Returns the name
 */
int get_module_name( int slot, int mod_num );

/**
 * Get the outputs of a module
 * Returns the output module ids
 */
PoolIntArray get_module_outputs( int slot, int mod_num );

/**
 * Read up to samples_to_read samples from a given module
 * Returns the samples
 */
PoolIntArray get_module_scope2( int slot, int mod_num, int channels, int samples_to_read );

/**
 * Get the position of a module
 * Returns a 2 elements Array containing respectively x and y
 */
PoolIntArray get_module_xy( int slot, int mod_num );

/**
 * Get the number of modules in the slot
 * Returns the number of modules
 */
int get_number_of_modules( int slot );

/**
 * Get the number of controllers in a module
 * Returns the number of controllers
 */
int get_number_of_module_ctls( int slot, int mod_num );

/**
 * Get the number of patterns in a slot
 * Returns the number of patterns
 */
int get_number_of_patterns( int slot );

/**
 * Retrieve the notes in a pattern as a dictionary:
 * - int note: 0 = nothing, 1 = C0, 25 = C2 (+- 12 per octave)
 * - int vel: 0 = default, 1..129 - velocity
 * - int module: the module number
 * - int ctl: 1..127 - controller number + 1
 * - int effect: effect
 * - int ctl_val: 0xXXYY value of controller or effect
 * Returns a dictionary containing the attributes: 
 */
int get_pattern_data( int slot, int pat_num );

/**
 * Get the number of lines in a pattern
 * Returns the number of lines
 */
int get_pattern_lines( int slot, int pat_num );

/**
 * Get the number of tracks in a pattern
 * Returns the number of tracks
 */
int get_pattern_tracks( int slot, int pat_num );

/**
 * Get the x coordinates of a pattern
 * Returns x
 */
int get_pattern_x( int slot, int pat_num );

/**
 * Get the y coordinates of a pattern
 * Returns y
 */
int get_pattern_y( int slot, int pat_num );

/**
 * Get the BPM of the song
 * Returns the beats per minute
 */
int get_song_bpm( int slot );

/**
 * Get the song's length in frames
 * Returns the length
 */
int get_song_length_frames( int slot );

/**
 * Get the song's length in lines
 * Returns the length
 */
int get_song_length_lines( int slot );

/**
 * Get the song's name
 * Returns the name
 */
int get_song_name( int slot );

/**
 * Get the song's ticks per line
 * Returns number of ticks per line
 */
int get_song_tpl( int slot );

/**
 * Get Sunvox's current ticks
 * Returns the ticks
 */
int get_ticks();

/**
 * Get Sunvox's current ticks per second
 * Returns the number of ticks per second
 */
int get_ticks_per_second();

/**
 * Initialize Sunvox with the following parameters:
 * config - additional configuration in the following format: "option_name=value|option_name=value";
 *          example: "buffer=1024|audiodriver=alsa|audiodevice=hw:0,0";
 *          use empty if you agree to the automatic configuration;
 * freq - sample rate (Hz); min - 44100;
 * channels - only 2 supported now;
 * flags - mix of the SV_INIT_FLAG_xxx flags:
 *   SV_INIT_FLAG_NO_DEBUG_OUTPUT     ( 1 << 0 )
 *   SV_INIT_FLAG_USER_AUDIO_CALLBACK ( 1 << 1 ) // Interaction with sound card is on the user side
 *   SV_INIT_FLAG_AUDIO_INT16         ( 1 << 2 )
 *   SV_INIT_FLAG_AUDIO_FLOAT32       ( 1 << 3 )
 *   SV_INIT_FLAG_ONE_THREAD          ( 1 << 4 ) // Audio callback and song modification functions are in single thread
 * Returns true if init succeeded, false otherwise
 */
bool init( String config_str, int frequency, int channels, int flags );

/**
 * Load a Sunvox project from a file path
 * Returns whatever Sunvox returns
 */
int load( int slot, String file_name );

/**
 * Load a Sunvox project from raw data
 * Returns whatever Sunvox returns
 */
int load_from_memory( int slot, PoolByteArray data );

/**
 * Load a Sunvox module from a file path
 * Returns whatever Sunvox returns
 */
int load_module( int slot, String file_name, int x, int y, int z );

/**
 * Load a Sunvox module from raw data
 * Returns whatever Sunvox returns
 */
int load_module_from_memory( int slot, PoolByteArray data, int x, int y, int z );

/**
 * Lock a slot - Necessary for functions requiring USE LOCK/UNLOCK!
 * Returns whatever Sunvox returns
 */
int lock_slot( int slot );

/**
 * USE LOCK/UNLOCK!
 * Create a new module
 * Returns the module number
 */
int sv_new_module( int slot, String type, String name, int x, int y, int z );

/**
 * USE LOCK/UNLOCK!
 * Remove a module
 * Returns whatever Sunvox returns
 */
int sv_remove_module( int slot, int mod_num );

/**
 * Manually open a slot
 * Returns whatever Sunvox returns
 */
int open_slot( int slot );

/**
 * USE LOCK/UNLOCK!
 * Mute a pattern
 * Returns whatever Sunvox returns
 */
int pattern_mute( int slot, int pat_num );

/**
 * Play from current position
 * Returns whatever Sunvox returns
 */
int play( int slot );

/**
 * Play from beginning of the song
 * Returns whatever Sunvox returns
 */
int play_from_beginning( int slot );

/**
 * Set cursor to given line number
 * Returns whatever Sunvox returns
 */
int rewind( int slot, int line_num );

/**
 * Load a sample in a sampler module from a file
 * Returns whatever Sunvox returns
 */
int sampler_load( int slot, int sampler_module, String file_name, int sample_slot );

/**
 * Load a sample in a sampler module from raw data
 * Returns whatever Sunvox returns
 */
int sampler_load_from_memory( int slot, int sampler_module, PoolByteArray data, int sample_slot );

/**
 * Send a Sunvox event
 * Returns whatever Sunvox returns
 */
int send_event( int slot, int track_num, int note, int vel, int module, int ctl, int effect, int ctl_val );

/**
 * Set autostop - whether the playback should stop or loop when the end of the song is reached
 * Returns whatever Sunvox returns
 */
int set_autostop( int slot );

/**
 * Stop playback
 * Returns whatever Sunvox returns
 */
int stop( int slot );

/**
 * Unlock a slot
 * Returns whatever Sunvox returns
 */
int unlock_slot( int slot );

/**
 * Handle input ON/OFF requests to enable/disable input ports of the sound card
 * (for example, after the Input module creation).
 * Returns whatever Sunvox returns
 */
int update_input();

/**
 * Set the volume 0..256
 * Returns whatever Sunvox returns
 */
int volume( int slot, int vol );
```

## Credits

Massive thanks to Alexander Zolotov for making Sunvox and to the Godot team for their amazing work!
