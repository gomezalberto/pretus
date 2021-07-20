import sys, getopt, numpy as np
import cv2
import worker

def image_loader(image_name):
    """load image, returns numpy tensor"""
    image = cv2.imread(image_name)
    image= cv2.cvtColor(image, cv2.COLOR_RGB2GRAY)

    return image

def main(argv):

    inputfile = ''
    outputfile = ''
    try:
        opts, args = getopt.getopt(argv,"hi:o:s:d:")
    except getopt.GetoptError:
        print('test.py -i <inputfile> -o <outputfile> -s <sigma in mm> -d <delay in sec>')
        sys.exit(2)
    if len(opts) < 4:
        print('Too few arguments {:d} found'.format(len(opts) ))
        print('test.py -i <inputfile> -o <outputfile> -s <sigma in mm> -d <delay in sec>')
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print('test.py -i <inputfile> -o <outputfile> -s <sigma in mm> -d <delay in sec>')
            sys.exit()
        elif opt in ("-i"):
            inputfile = arg
        elif opt in ("-o"):
            outputfile = arg
        elif opt in ("-s"):
            sigma = float(arg)
        elif opt in ("-d"):
            delay = float(arg)

    print('Input file is '+ inputfile)
    print('Output file is '+ outputfile)
    print('Sigma is {}'.format(sigma))
    print('Delay is {}'.format(delay))

    im_file = 'test_image.png'
    out_file = 'test_image_processed.png'

    # Load Testing image
    image = image_loader(inputfile)

    worker.initialize('this is the python folder')

    output = worker.dowork(image, sigma, delay)

    cv2.imwrite(outputfile, output)

if __name__ == '__main__':
    main(sys.argv[1:])

