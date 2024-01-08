import numpy as np
import tensorflow as tf
from keras.layers import Conv2D, MaxPooling2D, Flatten, Dense, Reshape, Conv2DTranspose
from sklearn.model_selection import train_test_split
import args
import sys

MAX_TRAINING_IMAGES = 60000 # Number of images to use for training
MAX_QUERY_IMAGES = 10000    # Number of images to use for query

if __name__ == "__main__":

    # Get files
    files = args.get_files(sys.argv)

    train_data = args.load_file(files["-d"])

    query_data = args.load_file(files["-q"])

    num_of_rows = train_data['number_of_rows']
    num_of_columns = train_data['number_of_columns']
    
    # Extract image train_data
    images = train_data['images'][:MAX_TRAINING_IMAGES]

    query_images = query_data['images'][:MAX_QUERY_IMAGES]

    # Reshape images to 4D tensor (batch_size, height, width, channels)
    images = images.reshape(-1, num_of_rows, num_of_columns, 1)

    query_images = query_images.reshape(-1, num_of_rows, num_of_columns, 1)

    train_images, val_images = train_test_split(images, test_size=0.1)

    # Normalize images to [0, 1] range
    train_images = train_images.astype('float32') / 255
    val_images = val_images.astype('float32') / 255

    latent_dim = 10

    # Define the encoder part of the model
    encoder = tf.keras.Sequential([
        Conv2D(32, (3, 3), activation='relu', padding='same', input_shape=(num_of_rows, num_of_columns, 1)),
        MaxPooling2D((2, 2), padding='same'),
        Conv2D(64, (3, 3), activation='relu', padding='same'),
        MaxPooling2D((2, 2), padding='same'),
        Flatten(),
        Dense(64, activation='relu'),
        Dense(latent_dim, activation='relu')
    ])

    # Define the decoder part of the model
    decoder = tf.keras.Sequential([
        Dense(64, activation='relu', input_shape=(10,)),
        Dense(7*7*64, activation='relu'),
        Reshape((7, 7, 64)),
        Conv2DTranspose(64, (3, 3), strides=2, activation='relu', padding='same'),
        Conv2DTranspose(32, (3, 3), strides=2, activation='relu', padding='same'),
        Conv2D(1, (3, 3), activation='relu', padding='same')
    ])

    # Combine the encoder and decoder into an autoencoder model
    autoencoder = tf.keras.Model(inputs=encoder.input, outputs=decoder(encoder.output))

    autoencoder.summary()

    autoencoder.compile(optimizer='adam', loss='mse', metrics=['mae'])

    # Train the model
    autoencoder.fit(train_images, train_images, epochs=5, batch_size=64, validation_data=(val_images, val_images))

    # Compress the images using the encoder
    compressed_train_images = encoder.predict(images)
    max_value_train = tf.reduce_max(compressed_train_images)

    compressed_query_images = encoder.predict(query_images)
    max_value_query = tf.reduce_max(compressed_query_images)

    min_value_train = tf.reduce_min(compressed_train_images)

    print("max value: ", max_value_train)
    print("min value: ", min_value_train)

    # Rescale the compressed images back to the 0-255 range
    compressed_train_images = (compressed_train_images / max_value_train) * 255
    compressed_train_images = tf.cast(compressed_train_images, tf.uint8)
    compressed_query_images = (compressed_query_images / max_value_query) * 255
    compressed_query_images = tf.cast(compressed_query_images, tf.uint8)

    train_data['number_of_images'] = MAX_TRAINING_IMAGES
    train_data['number_of_rows'] = latent_dim
    train_data['number_of_columns'] = 1
    train_data['images'] = compressed_train_images

    query_data['number_of_images'] = MAX_QUERY_IMAGES
    query_data['number_of_rows'] = latent_dim
    query_data['number_of_columns'] = 1
    query_data['images'] = compressed_query_images

    # Save the new images
    args.save_file(train_data, files["-od"])
    args.save_file(query_data, files["-oq"])