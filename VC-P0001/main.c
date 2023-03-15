#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "vc.h"
#include <stdlib.h>




//Abrir imagem, alterar e gravar em novo ficheiro
//VC03_19 INVERTER CORES ---image->data[i] = 255 - image->data[i]---;
int main_VC03_19(void)
{
	IVC* image;
	int i;

	image = vc_read_image("images/FLIR/flir-05.pgm");
	if (image == NULL)
	{
		printf("ERRO -> vc_read_image():\n\tFile not found!\n");
		getchar();
		return 0;
	}

	for (i = 0; i < image->bytesperline * image->height; i += image->channels)
	{
		image->data[i] = 255 - image->data[i];
	}

	vc_write_image("vc-0001.pgm", image);

	vc_image_free(image);

	printf("Press any key to exit...\n");
	getchar();

	return 0;
}
//CRIA IMAGEM
//WHITE-BLACK
//BLACK-WHITE
//VC03_24 
int main_VC03_24(void)
{
	IVC* image;
	int x, y;
	long int pos;

	image = vc_image_new(320, 280, 1, 1);
	if (image == NULL)
	{
		printf("ERRO -> vc_image_new():\n\tOut of memory!\n");
		getchar();
		return 0;
	}

	for (x = 0; x < image->width; x++)
	{
		for (y = 0; y < image->height; y++)
		{
			pos = y * image->bytesperline + x * image->channels;

			if ((x <= image->width / 2) && (y <= image->height / 2)) image->data[pos] = 1;
			else if ((x > image->width / 2) && (y > image->height / 2)) image->data[pos] = 1;
			else image->data[pos] = 0;
		}
	}

	vc_write_image("vc-0001.pgm", image);

	vc_image_free(image);

	printf("Press any key to exit...\n");
	getchar();

	return 0;
}

//ABRE IMAGEM BINARIA E TROCA PRETO POR BRANCO, GRAVA
//VC03_26
int main_VC03_26(void)
{
	IVC* image;
	int i;

	image = vc_read_image("vc-0001.pgm");
	if (image == NULL)
	{
		printf("ERRO -> vc_read_image():\n\tFile not found!\n");
		getchar();
		return 0;
	}

	for (i = 0; i < image->bytesperline * image->height; i += image->channels)
	{
		if (image->data[i] == 1) image->data[i] = 0;
		else image->data[i] = 1;
	}

	vc_write_image("vc-0002.pgm", image);

	vc_image_free(image);

	printf("Press any key to exit...\n");
	getchar();

	return 0;
}

//ABRE IMAGEM, GERA NEGATIVO GRAY, GRAVA
//ABRE IMAGEM, GERA NEGATIVO RGB, GRAVA
// GERA RGB GRAYS OU RGB SÓ COM 1 CANAL DE COR ATIVO
//VC_04_15/16/20
int main_VC_04_15_16_20(void)
{
	IVC* image;
	int i;

	//lê imagem
	image = vc_read_image("images/Additional/fruits.ppm");
	if (image == NULL)
	{
		printf("ERRO -> vc_read_image():\n\tFile not found!\n");
		getchar();
		return 0;
	}
	//vc_gray_negative(image); // PARA negativo de imagens GRAY
	//vc_rgb_negative(image); // PARA negativo de imagens RGB

	//troca todos para RED, GREE OU BLUE
	// 
	//vc_rgb_get_red_gray(image); // PARA RED 
	//vc_rgb_get_green_gray(image); // PARA GREEN 
	//vc_rgb_get_blue_gray(image); // PARA BLUE 

	//troca para RED, GREEN ou BLUE para 0
	// 
	//vc_rgb_get_red(image); // PARA RED 
	//vc_rgb_get_green(image); // PARA GREEN 
	vc_rgb_get_blue(image); // PARA BLUE 
	vc_write_image("0420B.ppm", image);

	//limpa memoria
	vc_image_free(image);

	printf("Press any key to exit...\n");
	getchar();

	return 0;
}

//VC_04_23
int main04_23(void)
{
	IVC* image;
	int i;

	//lê imagem
	image = vc_read_image("images/Additional/fruits.ppm");
	if (image == NULL)
	{
		printf("ERRO -> vc_read_image():\n\tFile not found!\n");
		getchar();
		return 0;
	}
	IVC* resultado;
	resultado = vc_image_new(image->width, image->height, 1, image->levels);


	vc_rgb_to_gray(image, resultado);
	vc_write_image("fruits2.ppm", resultado);


	//limpa memoria
	vc_image_free(image);
	vc_image_free(resultado);

	// Windows
	system("cmd start /c/FilterGear images/Additional/fruits.ppm"); // Input
	system("cmd start /c/FilterGear fruits2.ppm"); // Output

	printf("Press any key to exit...\n");
	getchar();

	return 0;
}

//VC_04_33
int main04_33(void)
{
	IVC* image;
	int i;


	//lê imagem
	image = vc_read_image("HSVTestImage01.ppm");
	if (image == NULL)
	{
		printf("ERRO -> vc_read_image():\n\tFile not found!\n");
		getchar();
		return 0;
	}
	IVC* resultado;
	resultado = vc_image_new(image->width, image->height, image->channels, image->levels);

	//vc_rgb_to_gray(image, resultado);
	vc_rgb_to_hsv(image, resultado);
	vc_write_image("HSVIMAGE.ppm", resultado);


	//limpa memoria
	vc_image_free(image);
	vc_image_free(resultado);

	// Windows
	system("cmd start /c/FilterGear HSVTestImage01.ppm"); // Input
	system("cmd start /c/FilterGear HSVIMAGE.ppm"); // Output

	printf("Press any key to exit...\n");
	getchar();

	return 0;
}

//VC_04_37
int main04_37(void)
{
	IVC* image;
	int i;


	//lê imagem
	image = vc_read_image("HSVIMAGE.ppm");
	if (image == NULL)
	{
		printf("ERRO -> vc_read_image():\n\tFile not found!\n");
		getchar();
		return 0;
	}
	IVC* resultado;
	resultado = vc_image_new(image->width, image->height, 1, image->levels);

	vc_hsv_segmentation(image, resultado, 55, 65, 49, 101, 59, 101);
	vc_write_image("HSVIMAGEBW.pgm", resultado);


	//limpa memoria
	vc_image_free(image);
	vc_image_free(resultado);

	// Windows
	system("cmd start /c/FilterGear HSVIMAGE.ppm"); // Input
	system("cmd start /c/FilterGear HSVIMAGEBW.pgm"); // Output

	printf("Press any key to exit...\n");
	getchar();

	return 0;
}

//VC04_42
int main(void)
{
	IVC* image;
	int i;


	//lê imagem
	image = vc_read_image("images/FLIR/flir-04.pgm");
	if (image == NULL)
	{
		printf("ERRO -> vc_read_image():\n\tFile not found!\n");
		getchar();
		return 0;
	}
	IVC* resultado;
	resultado = vc_image_new(image->width, image->height, 3, image->levels);

	vc_scale_gray_to_rgb(image, resultado);
	vc_write_image("flir-04RGB.ppm", resultado);


	//limpa memoria
	vc_image_free(image);
	vc_image_free(resultado);

	// Windows
	system("cmd start /c/FilterGear images/FLIR/flir-04.pgm"); // Input
	system("cmd start /c/FilterGear flir-04RGB.ppm"); // Output

	printf("Press any key to exit...\n");
	getchar();

	return 0;
}

