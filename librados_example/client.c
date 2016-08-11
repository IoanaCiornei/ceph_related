#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rados/librados.h>

#define TEXT "text to be sent"
#define TEXT_LENGTH 15

int read_object(rados_ioctx_t io, char *obj_name)
{
	rados_completion_t comp;
	char text_read[TEXT_LENGTH];
	int err;

	err = rados_aio_create_completion(NULL, NULL, NULL, &comp);
	if (err < 0) {
		fprintf(stderr, "Could not create aio completion\n");
		return err;
	}


	err = rados_read(io, obj_name, text_read, TEXT_LENGTH, 0);
	if (err < 0) {
		fprintf(stderr, "Could not schedule aio read\n");
		return err;
	}

	rados_aio_release(comp);

	printf("%s\n", text_read);

	return 0;
}

int write_object(rados_ioctx_t io, char *obj_name)
{
	int err;

	err = rados_write(io, obj_name, TEXT, TEXT_LENGTH, 0);
	if (err) {
		fprintf(stderr, "Cannot write object to pool\n");
		return err;
	}

	return 0;
}

int delete_object(rados_ioctx_t io, char *obj_name)
{
	int err;

	err = rados_remove(io, obj_name);
	if (err) {
		fprintf(stderr, "Cannot remove object from pool\n");
		return err;
	}

	return 0;

}

void print_usage(void)
{
	printf("Usage: ./client <operation> <num_objs> \n");
	printf("\t operation = read|write|rm\n");
	printf("\t num_objs - number of objects to read|write|delete\n");
}

int main(int argc, const char **argv)
{
	/* Declare the cluster handle and args */
	rados_t cluster;
	char cluster_name[] = "ceph";
	char user_name[] = "client.admin";
	char pool_name[] = "rbd";
	rados_ioctx_t io;
	uint64_t flags;
	int err, i, num_obj;
	char obj_name[10];
	char operation[10];


	if (argc != 3) {
		print_usage();
		exit(EXIT_FAILURE);
	}
	sprintf(operation, "%s", argv[1]);
	if ( strcmp(operation, "read") != 0 &&
		strcmp(operation, "write") != 0 &&
		strcmp(operation, "rm") != 0) {
		print_usage();
		exit(EXIT_FAILURE);
	}

	/* init cluster handle */
	err = rados_create2(&cluster, cluster_name, user_name, flags);
	if (err) {
		fprintf(stderr, "%s: Couldn't create the cluster handle!\n", argv[0]);
		exit(EXIT_FAILURE);
	} else {
		printf("Created a cluster handle!\n");
	}

	/* read ceph configuration file */
	err = rados_conf_read_file(cluster, "ceph.conf");
	if (err) {
		fprintf(stderr, "%s: Cannot read config file\n", argv[0]);
		exit(EXIT_FAILURE);
	} else {
		printf("Read configuration file !\n");
	}

	err = rados_conf_parse_argv(cluster, argc, argv);
	if (err) {
		fprintf(stderr, "%s: cannot parse command line arguments\n", argv[0]);
		exit(EXIT_FAILURE);
	} else {
		printf("Read command line arguments!\n");
	}

	/* connect to the cluster */
	err = rados_connect(cluster);
	if (err) {
		fprintf(stderr, "%s: cannot connect to the cluster\n", argv[0]);
		exit(EXIT_FAILURE);
	} else {
		printf("Connected to the cluster!\n");
	}

	err = rados_ioctx_create(cluster, pool_name, &io);
	if (err) {
		fprintf(stderr, "%s: cannot open rados pool\n", argv[0]);
		rados_shutdown(cluster);
		exit(EXIT_FAILURE);
	} else {
		printf("Created I/O context \n");
	}

	num_obj = atoi(argv[2]);
	for (i = 0; i < num_obj; i++) {
		sprintf(obj_name, "obj-%d", i);

		if (strcmp(operation, "write") == 0)
			err = write_object(io, obj_name);
		else if (strcmp(operation, "read") == 0)
			err = read_object(io, obj_name);
		else if (strcmp(operation, "rm") == 0)
			err = delete_object(io, obj_name);
	}

	rados_ioctx_destroy(io);
	rados_shutdown(cluster);

	return err;
}
