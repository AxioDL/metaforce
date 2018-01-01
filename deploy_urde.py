import urllib.request, json, sys
req = urllib.request.Request(
    "https://gitlab.axiodl.com/api/v4/projects/AxioDL%%2Furde/pipelines/%s/jobs?scope=success" % sys.argv[2],
    headers={'JOB-TOKEN':sys.argv[1]})
with urllib.request.urlopen(req) as url:
    data = json.loads(url.read().decode())
    indexes = {}
    for d in data:
        id = d['id']
        artifacts_file = d['artifacts_file']['filename']
        platform = d['name'].split(':')[1]
        file_req = urllib.request.Request(
            "https://gitlab.axiodl.com/AxioDL/urde/-/jobs/%d/artifacts/download" % id,
            headers={'JOB-TOKEN':sys.argv[1]})
        with urllib.request.urlopen(file_req) as url_file:
            bin_data = url_file.read()
            if platform not in indexes:
                indexes[platform] = []
            indexes[platform].append(artifacts_file)
            with open("/srv/releases/%s/%s" % (platform, artifacts_file), 'wb') as fout:
                fout.write(bin_data)
    for p in indexes:
        with open("/srv/releases/%s/index.txt" % p, 'w') as fout:
            for i in indexes[p]:
                fout.write(i + '\n')

