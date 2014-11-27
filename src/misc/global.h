#ifndef __NYX_GLOBAL_H__
#define __NYX_GLOBAL_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/* Debug flag */
#define NYX_DEBUG 1

/* Perform aligned memory allocations */
#define NYX_USE_ALIGNED_ALLOCATIONS 1
//#undef NYX_USE_ALIGNED_ALLOCATIONS


/* Error log macro */
#ifdef NYX_DEBUG
#define NYX_DLOG(...) fprintf(stdout, __VA_ARGS__)
#else
#define NYX_DLOG(...) ((void)0)
#define NDEBUG
#endif /* NYX_DEBUG */

#define NYX_ERRLOG(...) fprintf(stderr, __VA_ARGS__)


/* RGB pixel */
typedef struct _nyx_rgb_pixel_struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} rgb_pixel;

/* RGBA pixel */
typedef struct _nyx_rgba_pixel_struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
} rgba_pixel;

/* Image type */
typedef enum _nyx_img_type_t {
	img_type_tga = 1,
	img_type_jpg,
	img_type_png,
} img_type_t;

/* Color space */
typedef enum _nyx_colorspace_t {
	colorspace_unknown = 0,
	colorspace_rgb = 1,
	colorspace_rgba,
} colorspace_t;

/* Point */
typedef struct _nyx_point_struct {
	size_t x;
	size_t y;
} point;

/* Size */
typedef struct _nyx_size_struct {
	size_t w;
	size_t h;
} size;

/* Rect */
typedef struct _nyx_rect_struct {
	point origin;
	size size;
} rect;

/**
 * @brief Retrieve the number of components for a colorspace
 * @param colorspace [in] : the colorspace
 * @returns the number of components for this colorspace
 */
size_t nyx_num_components_for_colorspace(const colorspace_t colorspace);



/* Minimun value for a pixel component */
#define NYX_MIN_PIXEL_COMPONENT_VALUE ((uint8_t)0)
/* Maximum value for a pixel component */
#define NYX_MAX_PIXEL_COMPONENT_VALUE ((uint8_t)UINT8_MAX)
/* Returns the lower value */
#define NYX_MIN(A, B) ((A) < (B) ? (A) : (B))
/* Returns the higher value */
#define NYX_MAX(A, B) ((A) > (B) ? (A) : (B))
/* Clamp */
#define NYX_CLAMP(VAL, MIN, MAX) (((VAL) < (MIN)) ? (MIN) : (((VAL) > (MAX)) ? (MAX) : (VAL)))
/* Swap */
#define NYX_SWAP(__TYPE, A, B) do { __TYPE SWAP_TMP = B; B = A; A = SWAP_TMP; } while (0)
/* Returns a correct value for a pixel component (0 - 255) */
#define NYX_SAFE_PIXEL_COMPONENT_VALUE(COLOR) (NYX_CLAMP(COLOR, NYX_MIN_PIXEL_COMPONENT_VALUE, NYX_MAX_PIXEL_COMPONENT_VALUE))

/* RGBA manipulation */
#define NYX_RGBA_GET_R(RGBA) (RGBA & 0xFF)
#define NYX_RGBA_GET_G(RGBA) ((RGBA >> 8) & 0xFF)
#define NYX_RGBA_GET_B(RGBA) ((RGBA >> 16) & 0xFF)
#define NYX_RGBA_GET_A(RGBA) ((RGBA >> 24) & 0xFF)
#define NYX_RGBA_MAKE(R, G, B, A) ((A << 24) + (B << 16) + (G << 8) + R)

/* */
#define NYX_EQUAL_POINTS(P1, P2) ((P1.x == P2.x) && (P1.y == P2.y))
#define NYX_EQUAL_SIZES(S1, S2) ((S1.w == S2.w) && (S1.h == S2.h))
#define NYX_EQUAL_RECTS(R1, R2) ((NYX_EQUAL_POINTS(R1.origin, R2.origin)) && (NYX_EQUAL_SIZES(R1.size, R2.size)))


#endif /* __NYX_GLOBAL_H__ */
