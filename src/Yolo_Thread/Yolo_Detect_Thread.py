"""
@Author: Qu Xiangjun
@Time: 2021.02.23
@Describe: 此文件负责定义yolo检测的线程
"""

import argparse
from sys import platform
import threading
import numpy as np
import models
from models import *  # set ONNX_EXPORT in models.py
from utils.datasets import *
from utils.utils import *


class Yolo_Detect_Thread(threading.Thread):
    """
    目标检测线程
    """
    def __init__(self):
        threading.Thread.__init__(self)
        self.img_array = np.array([])
        self.result = []  # 目标检测结果，按照（类型，数量来划分）
        self.detect_img = np.array([])  # 有框图的目标检测结果

    def detect(self,save_txt=False, save_img=False):
        """
        目标检测函数
        """
        img0 = self.img_array

        # (320, 192) or (416, 256) or (608, 352) for (height, width)
        img_size = (320, 192) if ONNX_EXPORT else opt.img_size
        out, source, weights, half, view_img = opt.output, opt.source, opt.weights, opt.half, opt.view_img
        webcam = False  # source == '0' or source.startswith(
        # 'rtsp') or source.startswith('http') or source.endswith('.txt')

        # Initialize
        device = torch_utils.select_device(
            device='cpu' if ONNX_EXPORT else opt.device)
        # if os.path.exists(out): # out = /output  建立输出图片文件夹和先清除
        #     shutil.rmtree(out)  # delete output folder
        # os.makedirs(out)  # make new output folder

        # Initialize model
        model = models.Darknet(opt.cfg, img_size)

        # Load weights
        attempt_download(weights)
        if weights.endswith('.pt'):  # pytorch format
            model.load_state_dict(torch.load(
                weights, map_location=device)['model'])
        else:  # darknet format
            _ = load_darknet_weights(model, weights)

        # Second-stage classifier
        classify = False
        # if classify:
        #     modelc = torch_utils.load_classifier(
        #         name='resnet101', n=2)  # initialize
        #     modelc.load_state_dict(torch.load(
        #         'weights/resnet101.pt', map_location=device)['model'])  # load weights
        #     modelc.to(device).eval()

        # Fuse Conv2d + BatchNorm2d layers
        # model.fuse()

        # Eval mode
        model.to(device).eval()

        # Export mode
        if ONNX_EXPORT:
            img = torch.zeros((1, 3) + img_size)  # (1, 3, 320, 192)
            torch.onnx.export(
                model, img, 'weights/export.onnx', verbose=True)
            return

        # Half precision
        half = half and device.type != 'cpu'  # half precision only supported on CUDA
        if half:
            model.half()

        # Set Dataloader
        # vid_path, vid_writer = None, None
        # if webcam:  # 为网络流等
        #     view_img = True
        #     # set True to speed up constant image size inference
        #     torch.backends.cudnn.benchmark = True
        #     dataset = LoadStreams(source, img_size=img_size, half=half)
        # else:  # 为图片方式 加载图片数据
        #     save_img = True
        #     dataset = LoadImages(source, img_size=img_size, half=half)

        # Get classes and colors
        classes = load_classes("Yolo_Thread/"+parse_data_cfg(opt.data)['names'])
        colors = [[random.randint(0, 255) for _ in range(3)]
                  for _ in range(len(classes))]

        # Run inference
        t0 = time.time()
        # for path, img, im0s, vid_cap in dataset:

        # Padded resize
        im0s = img0
        img = letterbox(img0, new_shape=img_size)[0]
        # Normalize RGB
        img = img[:, :, ::-1].transpose(2, 0, 1)  # BGR to RGB
        img = np.ascontiguousarray(
            img, dtype=np.float16 if half else np.float32)  # uint8 to fp16/fp32
        img /= 255.0  # 0 - 255 to 0.0 - 1.0

        t = time.time()

        # Get detections
        img = torch.from_numpy(img).to(device)
        if img.ndimension() == 3:
            img = img.unsqueeze(0)
        pred = model(img)[0]

        if opt.half:
            pred = pred.float()

        # Apply NMS
        pred = non_max_suppression(pred, opt.conf_thres, opt.nms_thres)

        # Apply
        # if classify:
        #     pred = apply_classifier(pred, modelc, img, im0s)

        # Process detections
        for i, det in enumerate(pred):  # detections per image # i 为下标 det为值
            # if webcam:  # batch_size >= 1
            #     p, s, im0 = path[i], '%g: ' % i, im0s[i]
            # else:
            #     p, s, im0 = path, '', im0s
            im0 = im0s

            # save_path = str(Path(out) / Path(p).name) # 保存地址
            # s += '%gx%g ' % img.shape[2:]  # print string
            if det is not None and len(det):
                # Rescale boxes from img_size to im0 size
                det[:, :4] = scale_coords(
                    img.shape[2:], det[:, :4], im0.shape).round()

                self.result = []
                # Print results
                for c in det[:, -1].unique():
                    n = (det[:, -1] == c).sum()  # detections per class
                    # add to string  添加类别识别的数量和名称
                    self.result.append((n, classes[int(c)]))

                # Write results
                for *xyxy, conf, _, cls in det:
                    if save_txt:  # Write to file
                        with open(save_path + '.txt', 'a') as file:
                            file.write(('%g ' * 6 + '\n') %
                                       (*xyxy, cls, conf))

                    if save_img or view_img:  # Add bbox to image
                        label = '%s %.2f' % (classes[int(cls)], conf)  # 类型和概率
                        #plot_one_box(xyxy, im0, label=label, color=colors[int(cls)])
                        plot_one_box(xyxy, im0, label=classes[int(cls)],  # 画出来框在图片上
                                     color=colors[int(cls)])

            print('Done. (%.3fs)' % (time.time() - t))

            # Stream results
            # if view_img:
            cv2.imshow("imshow", im0)
            self.detect_img = im0

            # Save results (image with detections)
            # if save_img:
            #     if dataset.mode == 'images':
            #         cv2.imwrite(save_path, im0)
            #     else:
            #         if vid_path != save_path:  # new video
            #             vid_path = save_path
            #             if isinstance(vid_writer, cv2.VideoWriter):
            #                 vid_writer.release()  # release previous video writer

            #             fps = vid_cap.get(cv2.CAP_PROP_FPS)
            #             w = int(vid_cap.get(cv2.CAP_PROP_FRAME_WIDTH))
            #             h = int(vid_cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
            #             vid_writer = cv2.VideoWriter(
            #                 save_path, cv2.VideoWriter_fourcc(*opt.fourcc), fps, (w, h))
            #         vid_writer.write(im0)

        # if save_txt or save_img:
        #     print('Results saved to %s' % os.getcwd() + os.sep + out)
        #     if platform == 'darwin':  # MacOS
        #         os.system('open ' + out + ' ' + save_path)

        # print('Done. (%.3fs)' % (time.time() - t0))

    def run(self):
        """
        目标检测线程主函数
        """

        parser = argparse.ArgumentParser()
        parser.add_argument('--cfg', type=str,
                            default='Yolo_Thread/cfg/yolov3-spp.cfg', help='cfg file path')
        parser.add_argument('--data', type=str, default='Yolo_Thread/data/coco.data',
                            help='coco.data file path')  # 包含训练和测试的相关数据和类别等数据
        parser.add_argument(
            '--weights', type=str, default='Yolo_Thread/weights/yolov3-spp.weights', help='path to weights file')
        # input file/folder, 0 for webcam
        parser.add_argument('--source', type=str,
                            default='Yolo_Thread/data/samples', help='source')
        parser.add_argument('--output', type=str, default='Yolo_Threads/output',
                            help='output folder')  # output folder
        parser.add_argument('--img-size', type=int,
                            default=416, help='inference size (pixels)')
        parser.add_argument('--conf-thres', type=float,
                            default=0.3, help='object confidence threshold')
        parser.add_argument('--nms-thres', type=float, default=0.5,
                            help='iou threshold for non-maximum suppression')
        parser.add_argument('--fourcc', type=str, default='mp4v',
                            help='output video codec (verify ffmpeg support)')
        parser.add_argument('--half', action='store_true',
                            help='half precision FP16 inference')
        parser.add_argument('--device', default='',
                            help='device id (i.e. 0 or 0,1) or cpu')
        parser.add_argument(
            '--view-img', action='store_true', help='display results')
        global opt
        opt = parser.parse_args()
        print(opt)

        while(True):
            if(self.img_array is not None and len(self.img_array)):
                with torch.no_grad():
                    self.detect()
