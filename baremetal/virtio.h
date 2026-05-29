#ifndef VIRTIO_H
#define VIRTIO_H

void virtio_init(void);
int virtio_read_sector(int sector, char *buffer);
int virtio_write_sector(int sector, const char *buffer);

#endif