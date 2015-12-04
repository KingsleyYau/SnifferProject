package org.kk.snifferinstaller;

import org.kk.snifferinstaller.MovingImageView.Callback;
import org.kk.snifferinstaller.MovingImageView.TranslateMode;

import android.app.Activity;
import android.os.Bundle;
import android.view.animation.AlphaAnimation;
import android.widget.ImageView;

public class DefaultActivity extends Activity {
	
	public ImageView backImage;
	public MovingImageView imageView;
	
	private int[]  imgesResourceIds = new int[]{
			R.drawable.img_cool_360dp_width_1,
			R.drawable.img_cool_360dp_width_2,
			R.drawable.img_cool_360dp_width_3,
			R.drawable.long_gallery};
	
	private Images images = new Images(imgesResourceIds);
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		
		super.onCreate(savedInstanceState);
		InitView();
		animate();
	}
	
	@Override public void finish(){
		super.finish();
		overridePendingTransition(R.anim.anim_alpha_in_za, R.anim.anim_scale_out_az);  
	}
	
	@Override public void onDestroy(){
		imageView.stopAnimate();
		imageView.setKeepScreenOn(false);
		super.onDestroy();
	}

	@Override public void onPause(){
		imageView.stopAnimate();
		super.onPause();
	}
	
	@Override public void onResume(){
		imageView.runAnimate(imageView.mode);
		super.onResume();
	}
	
	public void InitView() {
		setContentView(R.layout.activity_default);
		
		backImage = (ImageView)findViewById(R.id.back_image);
		imageView = (MovingImageView) findViewById(R.id.imageView);
		
		final AlphaAnimation alAnimZA = new AlphaAnimation(1.0f, 0.0f);
		final AlphaAnimation alAnimAZ = new AlphaAnimation(0.0f, 1.0f);
		
		alAnimZA.setDuration(800);
		alAnimZA.setFillEnabled(true);
		alAnimZA.setFillAfter(true);
		
		imageView.setCallback(new Callback(){

			@Override
			public void onAnimationStopped() {
				// TODO Auto-generated method stub
				
//				//Don't repeat animation
//				if (images.getNextPosition() == images.getSize()){
//					return;
//				}
				
				//Last image use YTRANSLATE mode
				if (images.getNextPosition() == images.getSize() - 1){
					imageView.setMode(TranslateMode.YTRANSLATE);
				}else{
					imageView.setMode(TranslateMode.XTRANSLATE);
				}
				
				int nextImage = images.getNext();
				backImage.setImageResource(nextImage);
				imageView.setNextPhoto(nextImage);
			}
			
			@Override
			public void onBeforeSetNext() {
				// TODO Auto-generated method stub
				
//				text.startAnimation(alAnimAZ);
			}
			
			@Override
			public void onNextPhotoSet() {
				// TODO Auto-generated method stub
				imageView.runAnimate(imageView.mode, 0);
			}
			
		});

		//runAnimate();
		
	}
	
	private void animate(){
		imageView.setImageResource(images.getNext());
		imageView.setFrequency(10);
		imageView.setDuration(6000);
		imageView.runAnimate(TranslateMode.XTRANSLATE);
		imageView.setKeepScreenOn(true);
	}
	
	class Images{
		
		public int next = 0;
		public int[] images;
		
		public Images(int[] imageResourceIds){
			images = imageResourceIds;
		}
		
		public void moveToFirst(){
			next = 0;
		}
		
		public int getNext(){
			if (next > images.length - 1) next = 0;//throw  new Exception("No more images, please use moveToFirst().");
			int image =  images[next];
			next++;
			return image;
		}
		
		public int getNextPosition(){
			return next;
		}
		
		public int getSize(){
			return images.length;
		}
		
	}
}
