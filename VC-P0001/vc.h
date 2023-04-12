//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLITÉCNICO DO CÁVADO E DO AVE
//                          2022/2023
//             ENGENHARIA DE SISTEMAS INFORMÁTICOS
//                    VISÃO POR COMPUTADOR
//
//             [  BRUNO OLIVEIRA - boliveira@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#define VC_DEBUG


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


typedef struct {
	unsigned char *data;
	int width, height;
	int channels;			// Binário/Cinzentos=1; RGB=3
	int levels;				// Binário=1; Cinzentos [1,255]; RGB [1,255]
	int bytesperline;		// width * channels
} IVC;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROTÓTIPOS DE FUNÇÕES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// FUNÇÕES: ALOCAR E LIBERTAR UMA IMAGEM
IVC *vc_image_new(int width, int height, int channels, int levels);
IVC *vc_image_free(IVC *image);

// FUNÇÕES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
IVC *vc_read_image(char *filename);
int vc_write_image(char *filename, IVC *image);

// FUNÇÃO AULA ESPAÇOS DE COR VC04
int vc_gray_negative(IVC* srcdst);
int vc_rgb_negative(IVC* srcdst);
// VC04_20
int vc_rgb_get_red_gray(IVC* srcdst);
int vc_rgb_get_green_gray(IVC* srcdst);
int vc_rgb_get_blue_gray(IVC* srcdst);

int vc_rgb_get_red(IVC* srcdst);
int vc_rgb_get_green(IVC* srcdst);
int vc_rgb_get_blue(IVC* srcdst);

//VC04_23
int vc_rgb_to_gray(IVC* src, IVC* dst);

//VC04_33
int vc_rgb_to_hsv(IVC* src, IVC* dst);

//VC04_37
int vc_hsv_segmentation(IVC* src, IVC* dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax);
//VC04_42
int vc_scale_gray_to_rgb(IVC* src, IVC* dst);
//VC05_10
int vc_gray_to_binary(IVC* srcdst, int threshold);
//VC05_12
int vc_gray_to_binary_global_mean(IVC* src, IVC* dst);

//VC05_12_midpoint
int vc_gray_to_binary_kernel_midpoint(IVC* src, IVC* dst, int kernel);

//VC06_18 dilatacao erosao
int vc_binary_dilate(IVC* src, IVC* dst, int kernel);
//int vc_binary_erode(IVC* src, IVC* dst, int kernel);

//VC08_22 Histogramas gray show
int vc_gray_histogram_show(IVC* src, IVC* dst);

//VC08_22 Histogramas gray EQUALIZER
int vc_gray_histogram_equalization(IVC* src, IVC* dst);