#!/bin/sh

set -e

cd "$(dirname "$0")"

if [ "$1" = "help" ] || [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
	echo "Usage:"
	echo "  ./build_stress.sh [outer_rounds] [filter] [inner_stress_repeat]"
	echo
	echo "Defaults:"
	echo "  outer_rounds=5"
	echo "  filter=integration/stress/direct"
	echo "  inner_stress_repeat=use xpack_test default (5) unless provided"
	echo
	echo "Examples:"
	echo "  ./build_stress.sh"
	echo "  ./build_stress.sh 3"
	echo "  ./build_stress.sh 2 integration/stress/direct 6"
	echo "  ./build_stress.sh 1 all 2"
	echo
	echo "Notes:"
	echo "  This script is not parallel-safe."
	echo "  A lock file is created at release/x64/xpack_stress.lock during execution."
	echo "  A timestamped log is written to release/x64/xpack_stress_YYYYMMDD_HHMMSS.log"
	echo "  The latest run is also copied to release/x64/xpack_stress.log"
	echo "  A history row is appended to release/x64/xpack_stress_history.tsv"
	exit 0
fi

iRounds="$1"
if [ -z "$iRounds" ]; then
	iRounds=5
fi

sFilter="$2"
if [ -z "$sFilter" ]; then
	sFilter="integration/stress/direct"
fi

iInnerRepeat="$3"

sLogLatest="release/x64/xpack_stress.log"
sLock="release/x64/xpack_stress.lock"
sHistory="release/x64/xpack_stress_history.tsv"
sStamp="$(date +%Y%m%d_%H%M%S)"
sLog="release/x64/xpack_stress_${sStamp}.log"
iStartUnix="$(date +%s)"

mkdir -p release/x64

if [ -e "$sLock" ]; then
	echo "Stress lock exists: $sLock"
	echo "Another stress run is probably active."
	exit 3
fi

cleanup_lock()
{
	rm -f "$sLock"
}

trap cleanup_lock EXIT INT TERM

printf "%s %s\n" "$(date '+%Y-%m-%d')" "$(date '+%H:%M:%S')" > "$sLock"
printf "log=%s\n" "$sLog" >> "$sLock"

if [ ! -e "$sHistory" ]; then
	printf "stamp\tfilter\touter_rounds\tinner_stress_repeat\tresult\telapsed_seconds\tlog\n" > "$sHistory"
fi

./build_test.sh

{
	echo "xPack stress start"
	echo "stamp=$sStamp"
	echo "filter=$sFilter"
	echo "rounds=$iRounds"
	if [ -n "$iInnerRepeat" ]; then
		echo "stress_repeat=$iInnerRepeat"
	fi
	echo "start_date=$(date '+%Y-%m-%d')"
	echo "start_time=$(date '+%H:%M:%S')"
	echo "start_unix=$iStartUnix"
	echo
} > "$sLog"

echo "Running $iRounds round(s) with filter $sFilter ..."
if [ -n "$iInnerRepeat" ]; then
	echo "Inner stress repeat $iInnerRepeat"
fi

iRound=1
while [ "$iRound" -le "$iRounds" ]; do
	echo "[$iRound/$iRounds] $sFilter"
	echo "[$iRound/$iRounds] $sFilter" >> "$sLog"
	if [ -z "$iInnerRepeat" ]; then
		release/x64/xpack_test "$sFilter" >> "$sLog" 2>&1 || {
			iFailCode=$?
			iEndUnix="$(date +%s)"
			iElapsed=$((iEndUnix - iStartUnix))
			echo
			echo "Stress failed at round $iRound, exit code $iFailCode"
			{
				echo "FAILED round=$iRound exit=$iFailCode"
				echo "end_unix=$iEndUnix"
				echo "elapsed_seconds=$iElapsed"
			} >> "$sLog"
			cp "$sLog" "$sLogLatest"
			printf "%s\t%s\t%s\t%s\tFAILED_%s\t%s\t%s\n" "$sStamp" "$sFilter" "$iRounds" "$iInnerRepeat" "$iFailCode" "$iElapsed" "$sLog" >> "$sHistory"
			exit "$iFailCode"
		}
	else
		release/x64/xpack_test "$sFilter" "$iInnerRepeat" >> "$sLog" 2>&1 || {
			iFailCode=$?
			iEndUnix="$(date +%s)"
			iElapsed=$((iEndUnix - iStartUnix))
			echo
			echo "Stress failed at round $iRound, exit code $iFailCode"
			{
				echo "FAILED round=$iRound exit=$iFailCode"
				echo "end_unix=$iEndUnix"
				echo "elapsed_seconds=$iElapsed"
			} >> "$sLog"
			cp "$sLog" "$sLogLatest"
			printf "%s\t%s\t%s\t%s\tFAILED_%s\t%s\t%s\n" "$sStamp" "$sFilter" "$iRounds" "$iInnerRepeat" "$iFailCode" "$iElapsed" "$sLog" >> "$sHistory"
			exit "$iFailCode"
		}
	fi
	iRound=$((iRound + 1))
done

iEndUnix="$(date +%s)"
iElapsed=$((iEndUnix - iStartUnix))

echo
echo "Stress successful: $iRounds round(s)"
{
	echo "SUCCESS"
	echo "end_unix=$iEndUnix"
	echo "elapsed_seconds=$iElapsed"
} >> "$sLog"
cp "$sLog" "$sLogLatest"
printf "%s\t%s\t%s\t%s\tSUCCESS\t%s\t%s\n" "$sStamp" "$sFilter" "$iRounds" "$iInnerRepeat" "$iElapsed" "$sLog" >> "$sHistory"

exit 0
