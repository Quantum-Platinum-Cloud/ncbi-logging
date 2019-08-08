#!/bin/bash

# shellcheck source=/home/vartanianmh/strides/strides_env.sh
. "$HOME/strides/strides_env.sh"

export GOOGLE_APPLICATION_CREDENTIALS=/home/vartanianmh/nih-sra-datastore-c9b0ec6d9244.json
export CLOUDSDK_CORE_PROJECT="nih-sra-datastore"
gcloud auth activate-service-account --key-file=$GOOGLE_APPLICATION_CREDENTIALS

for LOG_BUCKET in "sra-pub-logs-1" "sra-ca-logs-1"; do
    echo "Processing $LOG_BUCKET"
    mkdir -p "/tmp/mike_logs/$LOG_BUCKET"
    gsutil -m rsync -r "gs://$LOG_BUCKET" "/tmp/mike_logs/$LOG_BUCKET"

    cd "/tmp/mike_logs/$LOG_BUCKET" || exit

    for OBJ in *usage*; do
    # sra-pub-run-1_usage_2019_06_24_03_00_00_07378a0db72e87e0b4_v0
        DT=${OBJ//_}
        DT=${DT:18:8}
        DT="/tmp/mike_logs/gs_prod/$DT"

        if [ ! -d "$DT" ]; then
            mkdir -p "$DT"
        fi
        if [ ! -e "$DT/$OBJ.gz" ]; then
            cp -n -v "$OBJ" "$DT"
            gzip -9 "$DT/$OBJ"
        fi

    done

#    ~/strides/gs_agent_all.py "$LOG_BUCKET" "SRA@GS"

    echo "Processed  $LOG_BUCKET"
done

export GOOGLE_APPLICATION_CREDENTIALS=/home/vartanianmh/requester-pays-key.json
export CLOUDSDK_CORE_PROJECT="research-sra-cloud-pipeline"
gcloud auth activate-service-account --key-file=$GOOGLE_APPLICATION_CREDENTIALS

for x in $(seq 7); do
    mkdir -p "$LOGDIR/gs_prod/objects"
    "$HOME/strides/gs_lister.py" "sra-pub-run-$x" | \
        gzip -9 -c > \
        "$LOGDIR/gs_prod/objects/$DATE.objects-$x.gz"
done


export GOOGLE_APPLICATION_CREDENTIALS=/home/vartanianmh/sandbox-blast-847af7ab431a.json
gcloud config set account 1008590670571-compute@developer.gserviceaccount.com
export CLOUDSDK_CORE_PROJECT="ncbi-sandbox-blast"


gsutil -m rsync -r /tmp/mike_logs/gs_prod \
    gs://strides_analytics/gs_prod

rsync -av /tmp/mike_logs/gs_prod/ "$PANFS/gs_prod/"

echo "Done"
date
