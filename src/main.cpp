#include "bn_core.h"
#include "bn_display.h"
#include "bn_keypad.h"
#include "bn_math.h"
#include "bn_random.h"
#include "bn_format.h"
#include "bn_blending.h"
#include "bn_sprite_text_generator.h"
#include "bn_regular_bg_actions.h"
#include "bn_regular_bg_builder.h"
#include "bn_regular_bg_attributes.h"

#include "common_info.h"
#include "common_variable_8x8_sprite_font.h"

#include "bn_regular_bg_items_title.h"
#include "bn_regular_bg_items_court.h"

#include "bn_sprite_items_paddle.h"
#include "bn_sprite_items_ball.h"
#include "bn_sprite_items_start.h"
#include "bn_sprite_items_press.h"

#include "bn_config_audio.h"
#include "bn_music_items.h"
#include "bn_sound_items.h"


namespace {

    bool check_collisions(bn::sprite_ptr& spriteA, bn::sprite_ptr& spriteB) {
        bool result =  spriteA.x() - spriteA.dimensions().width()/2 < spriteB.x() + spriteB.dimensions().width()/2 &&
            spriteA.x() + spriteA.dimensions().width()/2 > spriteB.x() - spriteB.dimensions().width()/2 &&
            spriteA.y() - spriteA.dimensions().height()/2 < spriteB.y() + spriteB.dimensions().height()/2 &&
            spriteA.y() + spriteA.dimensions().height()/2 > spriteB.y() - spriteB.dimensions().height()/2;
        return result;
    }

    void show_title() {
        bn::regular_bg_ptr title_bg = bn::regular_bg_items::title.create_bg(0, 0);
        double alpha = 0;
        
        bn::sprite_ptr press_sprite = bn::sprite_items::press.create_sprite(-32, 70);
        press_sprite.set_blending_enabled(true);
        bn::sprite_ptr start_sprite = bn::sprite_items::start.create_sprite(32, 70);
        start_sprite.set_blending_enabled(true);

        while (! bn::keypad::start_pressed()) {
            bn::blending::set_transparency_alpha(alpha);
            if (alpha < 1) {
                alpha = alpha + .01;
            }
            
            bn::core::update();
        }
        bn::sound_items::success.play(1);
        bn::core::update();

    }

    void game_loop() {
        bn::regular_bg_ptr court_bg = bn::regular_bg_items::court.create_bg(0, 0);
        bn::sprite_ptr ball_sprite = bn::sprite_items::ball.create_sprite(0, 0);
        bn::sprite_ptr paddle_1_sprite = bn::sprite_items::paddle.create_sprite(-110, 0);
        bn::sprite_ptr paddle_2_sprite = bn::sprite_items::paddle.create_sprite(110, 0);
        bn::sprite_text_generator small_variable_text_generator(common::variable_8x8_sprite_font);
        small_variable_text_generator.set_center_alignment();
        bn::vector<bn::sprite_ptr, 3> text_sprites;
        bn::vector<bn::sprite_ptr, 16> info_sprites;
        int delta_x = 0;
        int delta_y = 0;
        int player_1_points = 0;
        int player_2_points = 0;
        bool ball_moving = false;
        bn::random random;
        // Slows P2 down so he's not an invincible Pong God
        int p2_slowdown = 2;
        int p2_speed_loop = 0;

        while (true) {
            
            random.update();

            if(ball_moving) {
                info_sprites.clear();
            } else {
                info_sprites.clear();
                small_variable_text_generator.generate(0, 30, "PRESS A", info_sprites);
            }

            // Reset the ball position and shoot the ball when At is pressed
            if(bn::keypad::a_pressed() && ball_moving == false) {
                ball_sprite.set_y(0);
                ball_sprite.set_x(0);
                delta_x = 0;
                delta_y = 0;
                ball_moving = true;

                while (delta_x == 0 || delta_y == 0) {
                    delta_x = (random.get_int() % 5) - 2;
                    delta_y = (random.get_int() % 5) - 2;
                }
                bn::sound_items::bounce.play(1);
            }

            // Check to see if the ball hits the top and bottom walls
            if(ball_sprite.y() <= -76) {
                delta_y = delta_y * -1;
                bn::sound_items::bounce.play(1);
            }

            if(ball_sprite.y() >= 76) {
                delta_y = delta_y * -1;
                bn::sound_items::bounce.play(1);
            }

            // Check to see if a point is scored for P2
            // Reset the ball and update the score
            if(ball_sprite.x() - 4 <= -120) {
                player_2_points = player_2_points + 1;
                delta_y = 0;
                delta_x = 0;
                ball_sprite.set_y(0);
                ball_sprite.set_x(0);
                ball_moving = false;
                bn::sound_items::success.play(1);
            }

            // Check to see if a point is cored for P1
            // Reset the ball and update the score
            if(ball_sprite.x() + 4 >= 120) {
                player_1_points = player_1_points + 1;
                delta_y = 0;
                delta_x = 0;
                ball_sprite.set_y(0);
                ball_sprite.set_x(0);
                ball_moving = false;
                bn::sound_items::success.play(1);
            }

            // Move P2's paddle and limit it's maximum vertical movement
            if (p2_speed_loop < p2_slowdown) {
                p2_speed_loop = p2_speed_loop + 1;
            } else {

                if (paddle_2_sprite.y() > ball_sprite.y()) {
                    if(paddle_2_sprite.y() > -64) {
                        paddle_2_sprite.set_y(paddle_2_sprite.y() - 1);
                    }
                }

                if (paddle_2_sprite.y() < ball_sprite.y()) {
                    if(paddle_2_sprite.y() < 64) {
                        paddle_2_sprite.set_y(paddle_2_sprite.y() + 1);
                    }
                }
                p2_speed_loop = 0;
            }

            ball_sprite.set_y(ball_sprite.y() + delta_y);
            ball_sprite.set_x(ball_sprite.x() + delta_x);

            bool paddle_1_collision = check_collisions(paddle_1_sprite, ball_sprite);
            bool paddle_2_collision = check_collisions(paddle_2_sprite, ball_sprite);

            if (paddle_1_collision || paddle_2_collision) {
                delta_x = delta_x * -1;
                bn::sound_items::bounce.play(1);
            }

            text_sprites.clear();
            bn::string<32> p1_score = bn::to_string<32>(player_1_points);
            bn::string<32> p2_score = bn::to_string<32>(player_2_points);
            small_variable_text_generator.generate(-60, -70, p1_score, text_sprites);
            small_variable_text_generator.generate(60, -70, p2_score, text_sprites);

            // Move P1
            if(bn::keypad::up_held()) {
                if(paddle_1_sprite.y() > -64) {
                    paddle_1_sprite.set_y(paddle_1_sprite.y() - 2);
                }
            }
            else if(bn::keypad::down_held()) {
                if(paddle_1_sprite.y() < 64) {
                    paddle_1_sprite.set_y(paddle_1_sprite.y() + 2);
                }
            }

            bn::core::update();
        }
    }

}

int main() {
    
    
    bn::core::init();
    //bn::sound_items_info::span[0].first.play();
    bn::music_items::song.play(0.3);
    
    //bn::music_items_info::span[music_item_index].first.play(bn::fixed(music_volume) / 100);

    while (true) {

        show_title();
        bn::core::update();
        game_loop();
        bn::core::update();
       
    }
}
