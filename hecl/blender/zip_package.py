import sys, os
import zipfile

def zipdir(path, ziph):
    # ziph is zipfile handle
    for root, dirs, files in os.walk(path):
        for file in files:
            ziph.write(os.path.join(root, file))

package_path = 'hecl'
target_zip = 'hecl.zip'

zf = zipfile.ZipFile(target_zip, mode='w', compression=zipfile.ZIP_DEFLATED)
print('GENERATING', target_zip)
if os.path.isdir(package_path):
    zipdir(package_path, zf)

zf.close()
