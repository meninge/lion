
// Simple configuration
#define frames_nb 2
#define fsize     4
#define neu1      3
#define neu2      3

/* Dimensions for MNIST application
#define frames_nb 2
#define fsize     784
#define neu1      10
#define neu2      10
*/

// Config for neurons, layer 1
const uint32_t config_neu1[neu1][fsize] = {
	{ 0, 0, 0, 0},
	{ 0, 0, 0, 0},
	{ 0, 0, 0, 0}
};

// Config for bias/recode between layers 1-2
const uint32_t config_recode[neu1] = { 0, 0, 0};

// Config for neurons, layer 2
const uint32_t config_neu2[neu2][neu1] = {
	{ 0, 0, 0, 0},
	{ 0, 0, 0, 0},
	{ 0, 0, 0, 0}
};

// Frame data: 1000 frames of size 512
const uint32_t data_frames[frames_nb][fsize] = {
	{ 0, 0, 0, 0},
	{ 0, 0, 0, 0}
};
