import numpy as np
from tensorflow.keras import Model
from tensorflow.keras.layers import Input, Conv2D, BatchNormalization, MaxPooling2D, Flatten, Dense, Reshape, UpSampling2D
from tensorflow.keras import backend as K


def Encoder(encoder_input, num_conv_layers, conv_filters, conv_kernels, pool_sizes, latent_dim=10):
    x = encoder_input
    for i in range(num_conv_layers):
        x = Conv2D(conv_filters[i], conv_kernels[i], activation="relu", padding="same")(x)
        x = BatchNormalization()(x)
        x = MaxPooling2D(pool_sizes[i], padding="same")(x)
    shape_before = K.int_shape(x)[1:]
    x = Flatten()(x)
    return Dense(latent_dim)(x), shape_before


def Decoder(decoder_input, shape_before, num_conv_layers, conv_filters, conv_kernels, pool_sizes):
    x = Reshape(shape_before)(Dense(np.prod(shape_before))(decoder_input))
    for i in reversed(range(num_conv_layers)):
        x = Conv2D(conv_filters[i], conv_kernels[i], activation="relu", padding="same")(x)
        x = BatchNormalization()(x)
        x = UpSampling2D(pool_sizes[i])(x)
    x = Conv2D(1, conv_kernels[-1], activation="relu", padding="same")(x)
    return x


def Autoencoder(input_dim, num_conv_layers, conv_filters, conv_kernels, pool_sizes, latent_dim=10):
    encoder_input = Input(shape=input_dim)

    encoder_output, shape_before = Encoder(encoder_input, num_conv_layers, conv_filters, conv_kernels, pool_sizes, latent_dim)

    decoder_output = Decoder(encoder_output, shape_before, num_conv_layers, conv_filters, conv_kernels, pool_sizes)

    autoencoder = Model(encoder_input, decoder_output)

    return autoencoder
