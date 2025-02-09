(Block 50 - Blocks)

find blocks loaded?

3 load(strings)

block-sz 2048 ;inline
num-blocks500 ;inline

block-sz num-blocks * var blocks
num-blocks var flgs(clean/dirty flags)

blk-norm(n1--n2)0 maxnum-blocks 1- lit,min ;
blk-dirty!(n--)blk-norm 1 swap flgs + c! ;
blk-clean!(n--)blk-norm 0 swap flgs + c! ;
blk-dirty?(n--)blk-norm flgs + c@ ;
blk-buf(n--buf)blk-norm block-sz * blocks + ;

32 var fn(file-name)
blk-fn(n--fn)>r fn z" block-" s-cpy r>
    <# # # #s #> s-cat z" .fth" s-cat ;

blk-clear(n--)dup blk-clean! blk-buf block-sz 0 fill ;
blk-clear-all(--)num-blocks for i blk-clear next ;

51 load-next






