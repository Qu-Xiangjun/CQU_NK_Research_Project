Imports System.Runtime.InteropServices
Imports System.Threading.Thread
Imports System.Net.IPAddress
Imports MvCamCtrl.NET

Module ConnectSpecCamera

    Sub Main()
        Dim dev As MyCamera = New MyCamera
        Dim nRet As Int32 = MyCamera.MV_OK

        Dim stDevInfo As MyCamera.MV_CC_DEVICE_INFO = New MyCamera.MV_CC_DEVICE_INFO
        stDevInfo.nTLayerType = MyCamera.MV_GIGE_DEVICE
        Dim stGigEDev As MyCamera.MV_GIGE_DEVICE_INFO = New MyCamera.MV_GIGE_DEVICE_INFO

        ' ch:需要连接的相机ip(根据实际填充) | en:The camera IP that needs to be connected (based on actual padding)
        Console.WriteLine("Please input Device Ip : ")
        Dim strCurrentIp As String
        strCurrentIp = Console.ReadLine()
        ' ch:相机对应的网卡ip(根据实际填充) | en:The pc IP that needs to be connected (based on actual padding)
        Console.WriteLine("Please Net Export Ip : ")
        Dim strNetExport As String
        strNetExport = Console.ReadLine()
        Try
            Dim nIp1 As UInt32 = Convert.ToUInt32(Split(strCurrentIp, ".")(0))
            Dim nIp2 As UInt32 = Convert.ToUInt32(Split(strCurrentIp, ".")(1))
            Dim nIp3 As UInt32 = Convert.ToUInt32(Split(strCurrentIp, ".")(2))
            Dim nIp4 As UInt32 = Convert.ToUInt32(Split(strCurrentIp, ".")(3))
            stGigEDev.nCurrentIp = ((nIp1 << 24) Or (nIp2 << 16) Or (nIp3 << 8) Or nIp4)

            nIp1 = Convert.ToInt32(Split(strNetExport, ".")(0))
            nIp2 = Convert.ToInt32(Split(strNetExport, ".")(1))
            nIp3 = Convert.ToInt32(Split(strNetExport, ".")(2))
            nIp4 = Convert.ToInt32(Split(strNetExport, ".")(3))
            stGigEDev.nNetExport = ((nIp1 << 24) Or (nIp2 << 16) Or (nIp3 << 8) Or nIp4)
        Catch ex As Exception
            Console.WriteLine("Invalid input!")
            Console.WriteLine("push enter to exit")
            System.Console.ReadLine()
            End
        End Try
        Dim stGigeInfoPtr As IntPtr = Marshal.AllocHGlobal(216)
        Marshal.StructureToPtr(stGigEDev, stGigeInfoPtr, False)
        ReDim stDevInfo.SpecialInfo.stGigEInfo(539)
        Marshal.Copy(stGigeInfoPtr, stDevInfo.SpecialInfo.stGigEInfo, 0, 540)

        Do While (True)
            ' ch:创建句柄 | en:Create handle
            nRet = dev.MV_CC_CreateDevice_NET(stDevInfo)
            If 0 <> nRet Then
                Console.WriteLine("Create device failed:{0:x8}", nRet)
                Exit Do
            End If

            ' ch:打开相机 | en:Open devic
            nRet = dev.MV_CC_OpenDevice_NET()
            If 0 <> nRet Then
                Console.WriteLine("Open device failed:{0:x8}", nRet)
                Exit Do
            End If

            ' ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
            If stDevInfo.nTLayerType = MyCamera.MV_GIGE_DEVICE Then
                Dim nPacketSize As Int32
                nPacketSize = dev.MV_CC_GetOptimalPacketSize_NET()
                If nPacketSize > 0 Then
                    nRet = dev.MV_CC_SetIntValue_NET("GevSCPSPacketSize", nPacketSize)
                    If 0 <> nRet Then
                        Console.WriteLine("Warning: Set Packet Size failed:{0:x8}", nRet)
                    End If
                Else
                    Console.WriteLine("Warning: Get Packet Size failed:{0:x8}", nPacketSize)
                End If
            End If

            ' ch:开启取流 | en:Start grabbing
            nRet = dev.MV_CC_StartGrabbing_NET()
            If 0 <> nRet Then
                Console.WriteLine("Start grabbing fauled:{0:x8}", nRet)
                Exit Do
            End If

            Dim nCount As Int32 = 0
            Dim stFrameOut As MyCamera.MV_FRAME_OUT = New MyCamera.MV_FRAME_OUT

            ' ch:抓取图像 | en:Get image
            Do While nCount <> 10
                nCount = nCount + 1
                nRet = dev.MV_CC_GetImageBuffer_NET(stFrameOut, 1000)
                If MyCamera.MV_OK = nRet Then
                    Console.WriteLine("Width:" + Convert.ToString(stFrameOut.stFrameInfo.nWidth) + " Height:" + Convert.ToString(stFrameOut.stFrameInfo.nHeight) + " FrameNum:" + Convert.ToString(stFrameOut.stFrameInfo.nFrameNum))

                    Dim stSaveParam As MyCamera.MV_SAVE_IMG_TO_FILE_PARAM = New MyCamera.MV_SAVE_IMG_TO_FILE_PARAM()
                    stSaveParam.enImageType = MyCamera.MV_SAVE_IAMGE_TYPE.MV_Image_Bmp
                    stSaveParam.enPixelType = stFrameOut.stFrameInfo.enPixelType
                    stSaveParam.pData = stFrameOut.pBufAddr
                    stSaveParam.nDataLen = stFrameOut.stFrameInfo.nFrameLen
                    stSaveParam.nHeight = stFrameOut.stFrameInfo.nHeight
                    stSaveParam.nWidth = stFrameOut.stFrameInfo.nWidth
                    stSaveParam.iMethodValue = 2
                    stSaveParam.pImagePath = "Image_w" + stSaveParam.nWidth.ToString() + "_h" + stSaveParam.nHeight.ToString() + "_fn" + stFrameOut.stFrameInfo.nFrameNum.ToString() + ".bmp"
                    nRet = dev.MV_CC_SaveImageToFile_NET(stSaveParam)
                    If MyCamera.MV_OK <> nRet Then
                        Console.WriteLine("Save Image failed:{0:x8}", nRet)
                    End If
                    dev.MV_CC_FreeImageBuffer_NET(stFrameOut)
                    Continue Do
                Else
                    Console.WriteLine("Get Image failed:{0:x8}", nRet)
                End If
            Loop

            ' ch:停止取流 | en:Stop grabbing
            nRet = dev.MV_CC_StopGrabbing_NET()
            If 0 <> nRet Then
                Console.WriteLine("Stop Grabbing failed:{0:x8}", nRet)
                Exit Do
            End If

            ' ch:关闭相机 | en:Close device
            nRet = dev.MV_CC_CloseDevice_NET()
            If 0 <> nRet Then
                Console.WriteLine("Open device failed:{0:x8}", nRet)
                Exit Do
            End If

            ' ch:销毁句柄 | en:Destroy handle
            nRet = dev.MV_CC_DestroyDevice_NET()
            If 0 <> nRet Then
                Console.WriteLine("Destroy device failed:{0:x8}", nRet)
            End If

            Exit Do
        Loop

        If 0 <> nRet Then
            ' ch:销毁句柄 | en:Destroy handle
            nRet = dev.MV_CC_DestroyDevice_NET()
            If 0 <> nRet Then
                Console.WriteLine("Destroy device failed:{0:x8}", nRet)
            End If
        End If

        Console.WriteLine("push enter to exit")
        System.Console.ReadLine()
    End Sub

End Module
